#include "Characters/PartyCharacter.h"

#include "AbilitySystemComponent.h"
#include "DeathmatchParty.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PartyAttributeSet.h"
#include "GameMechanics/CombatComponent.h"
#include "GameMechanics/BuffComponent.h"
#include "Weapons/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameMechanics/LagCompensationComponent.h"
#include "GameMechanics/TeamPlayerStart.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerController/PartyPlayerController.h"
#include "GameModes//PartyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerState/PartyPlayerState.h"
#include "GameState/PartyGameState.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Weapons/Flag.h"

APartyCharacter::APartyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera =CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverhearWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverhearWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 56.f;
	MinNetUpdateFrequency = 33.f;

	TurnThreshold = 3.0f;

	ElimDelay = 3.0f;
	
	bLeftGame = false;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DIssolveTimelineComponent"));

	SetupCollisionBoxes();
}

void APartyCharacter::BeginPlay()
{
	Super::BeginPlay();

	PartyGameMode = GetWorld()->GetAuthGameMode<APartyGameMode>();
	
	if (IsValid(AbilitySystemComponent))
	{
		AttributeSet = AbilitySystemComponent->GetSet<UPartyAttributeSet>();
		BindAttributeChangeDelegates();
	}

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
}

void APartyCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	HideCharacterIfCharacterClose();
	
	RotateInPlace(DeltaTime);
}

UAbilitySystemComponent* APartyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APartyCharacter::SpawnDefaultWeapon()
{
	if (UWorld* World = GetWorld(); PartyGameMode && World && !bIsEliminated && DefaultWeapon)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeapon);
		if (CombatComponent)
		{
			CombatComponent->EquipWeapon(StartingWeapon);
			StartingWeapon->bDestroyWeapon = true;
		}
	}
}

ETeam APartyCharacter::GetTeam()
{
	PartyPlayerState = PartyPlayerState == nullptr ? GetPlayerState<APartyPlayerState>() : PartyPlayerState;

	if (PartyPlayerState == nullptr)
	{
		return ETeam::ET_NoTeam;
	}

	return PartyPlayerState->GetPlayerTeam();
}

void APartyCharacter::RotateInPlace(float DeltaTime)
{
	if (CombatComponent && CombatComponent->bHoldingFlag)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		return;
	}

	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
	}
	
	if (bDisabledGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void APartyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APartyCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &APartyCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APartyCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &APartyCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APartyCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &APartyCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &APartyCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APartyCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APartyCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APartyCharacter::ReloadButtonPressed);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &APartyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APartyCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MouseX", this, &APartyCharacter::MoveCameraX);
	PlayerInputComponent->BindAxis("MouseY", this, &APartyCharacter::MoveCameraY);
}

void APartyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->InitializePartyCharacter(this);
	}

	if (BuffComponent)
	{
		BuffComponent->Character = this;
		BuffComponent->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComponent->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}

	if (LagCompensationComponent)
	{
		LagCompensationComponent->PartyCharacter = this;
		
		if (Controller)
		{
			LagCompensationComponent->PartyController = Cast<APartyPlayerController>(Controller);
		}
	}
}

void APartyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APartyCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(APartyCharacter, bDisabledGameplay)
}

void APartyCharacter::OnRep_OverlappingWeapon(const AWeapon* LastWeapon) const
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->DisplayPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->DisplayPickupWidget(false);
	}
}

void APartyCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	
	SimProxisTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void APartyCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void APartyCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHUDHealth();

	if (Data.OldValue > Data.NewValue)
	{
		PlayHitReactionMontage();
	}
}

void APartyCharacter::OnShieldChanged(const FOnAttributeChangeData& Data)
{
	UpdateHUDShield();

	if (Data.OldValue > Data.NewValue)
	{
		PlayHitReactionMontage();
	}
}

void APartyCharacter::BindAttributeChangeDelegates()
{
	if (AbilitySystemComponent)
	{
		// Bind the delegate for health attribute changes
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &APartyCharacter::OnHealthChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetShieldAttribute()).AddUObject(this, &APartyCharacter::OnShieldChanged);
	}
}

void APartyCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ThisClass::UpdateDissolveMaterial);
	
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void APartyCharacter::MulticastGainTheLead_Implementation()
{
	if (CrownSystem == nullptr)
	{
		return;
	}

	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem.LoadSynchronous(),
			GetMesh(),
			FName(),
			GetActorLocation() + FVector(0.0f, 0.0f, 110.0f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false);
	}

	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
	
}

void APartyCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void APartyCharacter::SetSpawnPoint()
{
	if (HasAuthority() && PartyPlayerState->GetPlayerTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStartArray;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStartArray);

		if (PlayerStartArray.Num() > 0)
		{
			TArray<ATeamPlayerStart*> TeamPlayerStartArray;
			
			for (AActor* Start : PlayerStartArray)
			{
				if (ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start); TeamStart->Team == PartyPlayerState->GetPlayerTeam())
				{
					TeamPlayerStartArray.Add(TeamStart);
				}
			}
			
			if 	(TeamPlayerStartArray.Num() > 0)
			{
				int32 ArrayElement = FMath::RandRange(0, TeamPlayerStartArray.Num() - 1);
				ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStartArray[ArrayElement];
				SetActorLocationAndRotation(
					ChosenPlayerStart->GetActorLocation(),
					ChosenPlayerStart->GetActorRotation());
			}
		}
	}
}

void APartyCharacter::UpdateHUDHealth()
{
	CachedPartyPlayerController = CachedPartyPlayerController == nullptr ? Cast<APartyPlayerController>(Controller) : CachedPartyPlayerController;
	if (CachedPartyPlayerController && AttributeSet)
	{
		CachedPartyPlayerController->SetHUDHealth(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
	}
}

void APartyCharacter::UpdateHUDShield()
{
	CachedPartyPlayerController = CachedPartyPlayerController == nullptr ? Cast<APartyPlayerController>(Controller) : CachedPartyPlayerController;
	if (CachedPartyPlayerController && AttributeSet)
	{
		CachedPartyPlayerController->SetHUDShield(AttributeSet->GetShield(), AttributeSet->GetMaxShield());
	}
}

void APartyCharacter::UpdateHUDAmmo()
{
	CachedPartyPlayerController = CachedPartyPlayerController == nullptr ? Cast<APartyPlayerController>(Controller) : CachedPartyPlayerController;
	if (CachedPartyPlayerController && CombatComponent && CombatComponent->EquippedWeapon)
	{
		CachedPartyPlayerController->SetHUDCarriedAmmo(CombatComponent->CarriedAmmo);
		CachedPartyPlayerController->SetHUDWeaponAmmo(CombatComponent->EquippedWeapon->GetAmmo());
	}
}

AWeapon* APartyCharacter::GetEquippedWeapon() const
{
	if (CombatComponent == nullptr)
	{
		return nullptr;
	}

	return CombatComponent->EquippedWeapon;
}

ECombatState APartyCharacter::GetCombatState() const
{
	if (CombatComponent == nullptr)
	{
		return ECombatState::ECS_MAX;
	}
	
	return CombatComponent->CombatState;
}

void APartyCharacter::PlayFireMontage(bool inIsAiming) const
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && FireWeaponMontage)
	{
		const FName SectionName = inIsAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
		AnimInstance->Montage_Play(FireWeaponMontage);
	}
}

void APartyCharacter::PlayHitReactionMontage() const
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && HitReactionMontage)
	{
		const FName SectionName = FName("HitFront");
		AnimInstance->Montage_JumpToSection(SectionName);
		AnimInstance->Montage_Play(HitReactionMontage);
	}
}

void APartyCharacter::PlayEliminatedMontage() const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EliminatedMontage)
	{
		const FName SectionName = FName("Default");
		AnimInstance->Montage_JumpToSection(SectionName);
		AnimInstance->Montage_Play(EliminatedMontage);
	}
}

void APartyCharacter::PlayReloadMontage() const
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		FName SectionName;
		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
			case EWeaponType::EWT_AssaultRifle:
				SectionName = "Rifle";
				break;
			case EWeaponType::EWT_RocketLauncher:
				SectionName = "Rifle";
				break;
			case EWeaponType::EWT_Pistol:
				SectionName = "Rifle";
				break;
			case EWeaponType::EWT_Shotgun:
				SectionName = "Rifle";
				break;
			case EWeaponType::EWT_Sniper:
				SectionName = "Rifle";
				break;
			case EWeaponType::EWT_GranadeLauncher:
				SectionName = "Rifle";
				break;
			default:
				break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
		AnimInstance->Montage_Play(ReloadMontage);
	}
}

void APartyCharacter::PlaySwapMontage() const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapWeaponMontage)
	{
		AnimInstance->Montage_Play(SwapWeaponMontage);
	}
}

void APartyCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr)
	{
		return;
	}
	
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void APartyCharacter::DropOrDestroyWeapons()
{
	if (CombatComponent)
	{
		if (AWeapon* Weapon = CombatComponent->EquippedWeapon)
		{
			DropOrDestroyWeapon(Weapon);
		}

		if (AWeapon* BackupWeapon = CombatComponent->BackupWeapon)
		{
			DropOrDestroyWeapon(BackupWeapon);
		}
		
		if (AWeapon* Flag = CombatComponent->TheFlag)
		{
			Flag->Dropped();
		}
	}
}

void APartyCharacter::SetTeamColor(ETeam TeamSelected)
{
	if (GetMesh() == nullptr)
	{
		return;
	}
	
	switch (TeamSelected)
	{
	case ETeam::ET_NoTeam:
		if (OriginalMaterialInstance != nullptr)
		{
			GetMesh()->SetMaterial(0, OriginalMaterialInstance.LoadSynchronous());
			DissolvedMaterialInstance = BlueDissolveMaterialInstance.Get();
		}
		break;
	case ETeam::ET_BlueTeam:
		if (BlueMaterialInstance != nullptr)
		{
			GetMesh()->SetMaterial(0, BlueMaterialInstance.LoadSynchronous());
			DissolvedMaterialInstance = BlueDissolveMaterialInstance.LoadSynchronous();
		}
		break;
	case ETeam::ET_RedTeam:
		if (RedMaterialInstance != nullptr)
		{
			GetMesh()->SetMaterial(0, RedMaterialInstance.LoadSynchronous());
			DissolvedMaterialInstance = RedDissolveMaterialInstance.LoadSynchronous();
		}
		break;
	default:
		break;
	}
}

void APartyCharacter::Eliminated(bool bPlayerLeftGame)
{
	DropOrDestroyWeapons();
	
	MulticastEliminated(bPlayerLeftGame);
}

void APartyCharacter::ServerLeaveGame_Implementation()
{
	if (PartyGameMode)
	{
		PartyPlayerState = PartyPlayerState == nullptr ? GetPlayerState<APartyPlayerState>() : PartyPlayerState;
		PartyGameMode->PlayerLeftGame(PartyPlayerState);
	}
}

void APartyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	CachedPartyPlayerController =  Cast<APartyPlayerController>(NewController);

	SpawnDefaultWeapon();
	UpdateHUDAmmo();
}

void APartyCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);
	
	PartyPlayerState = GetPlayerState<APartyPlayerState>();
	if (PartyPlayerState)
	{
		PartyPlayerState->AddToScore(0);
		PartyPlayerState->AddToDefeats(0);
		SetTeamColor(PartyPlayerState->GetPlayerTeam());
		SetSpawnPoint();
	}

	if (const APartyGameState* PartyGameState = Cast<APartyGameState>(UGameplayStatics::GetGameState(this)))
	{
		if (PartyGameState->TopScoringPlayers.Contains(PartyPlayerState))
		{
			MulticastGainTheLead();
		}
	}
}

void APartyCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 75.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -75.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = InterpAO_Yaw;
		
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void APartyCharacter::HideCharacterIfCharacterClose() const
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->SetVisibility(false);
		}
		if (CombatComponent && CombatComponent->BackupWeapon && CombatComponent->BackupWeapon->GetWeaponMesh())
		{
			CombatComponent->BackupWeapon->GetWeaponMesh()->SetVisibility(false);
		}
		if (CombatComponent && CombatComponent->TheFlag)
		{
			Cast<AFlag>(CombatComponent->TheFlag)->SetFlagVisibility(false);
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->SetVisibility(true);
		}
		if (CombatComponent && CombatComponent->BackupWeapon && CombatComponent->BackupWeapon->GetWeaponMesh())
		{
			CombatComponent->BackupWeapon->GetWeaponMesh()->SetVisibility(true);
		}
		if (CombatComponent && CombatComponent->TheFlag)
		{
			Cast<AFlag>(CombatComponent->TheFlag)->SetFlagVisibility(true);
		}
	}
}

void APartyCharacter::EliminatedTimerFinished()
{
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
		return;	
	}

	if (PartyGameMode)
	{
		PartyGameMode->RequestRespawn(this, CachedPartyPlayerController);
	}
}

void APartyCharacter::MulticastEliminated_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	
	bIsEliminated = true;
	PlayEliminatedMontage();

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	if (CachedPartyPlayerController)
	{
		bDisabledGameplay = true;
		CachedPartyPlayerController->SetHUDWeaponAmmo(0);
	}

	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(false);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (DissolvedMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolvedMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), -0.55);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200);
	}
	
	StartDissolve();

	if (IsLocallyControlled() && IsAiming())
	{
		ShowSniperScopeWidget(false);
	}
	
	GetWorldTimerManager().SetTimer(
	EliminatedTimer,
	this,
	&ThisClass::EliminatedTimerFinished,
	ElimDelay);

	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

/**
 * INTERACTION SECTION
 */

void APartyCharacter::SetOverlappingWeapon(AWeapon* inWeapon)
{
	if (OverlappingWeapon && IsLocallyControlled())
	{
		OverlappingWeapon->DisplayPickupWidget(false);
	}
	
	OverlappingWeapon = inWeapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->DisplayPickupWidget(true);
		}
	}
}

bool APartyCharacter::IsWeaponEquipped() const
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

bool APartyCharacter::IsAiming() const
{
	return (CombatComponent && CombatComponent->bAiming);
}


void APartyCharacter::ServerEquippedButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		if (OverlappingWeapon)
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else if (CombatComponent->ShouldSwapWeapons())
		{
			CombatComponent->SwapWeapon();
		}
	}
}

/**
 * INPUT SECTION
 */

void APartyCharacter::MoveForward(float Value)
{
	if (bDisabledGameplay)
	{
		return;
	}
	
	if (Controller != nullptr && Value != 0)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));

		AddMovementInput(Direction, Value);
	}
}

void APartyCharacter::MoveRight(float Value)
{
	if (bDisabledGameplay)
	{
		return;
	}
	
	if (Controller != nullptr && Value != 0)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));

		AddMovementInput(Direction, Value);
	}
}

void APartyCharacter::MoveCameraX(float Value)
{
	if (Controller != nullptr && Value != 0)
	{
		AddControllerYawInput(Value);
	}
}

void APartyCharacter::MoveCameraY(float Value)
{
	if (Controller != nullptr && Value != 0)
	{
		AddControllerPitchInput(-Value);
	}
}

void APartyCharacter::EquipButtonPressed()
{
	if (bDisabledGameplay)
	{
		return;
	}
	
	if (CombatComponent)
	{
		if (CombatComponent->bHoldingFlag)
		{
			return;
		}
		
		if (CombatComponent->CombatState == ECombatState::ECS_Unoccupied)
		{
			ServerEquippedButtonPressed();
		}

		const bool bSwap = CombatComponent->ShouldSwapWeapons() &&
			!HasAuthority() &&
			CombatComponent->CombatState == ECombatState::ECS_Unoccupied &&
			OverlappingWeapon == nullptr;
		
		if (bSwap)
		{
			PlaySwapMontage();
			CombatComponent->CombatState = ECombatState::ECS_SwappingWeapon;
		}
	}
}

void APartyCharacter::CrouchButtonPressed()
{
	if (bDisabledGameplay)
	{
		return;
	}

	if (CombatComponent && CombatComponent->bHoldingFlag)
	{
		return;
	}
	
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void APartyCharacter::AimButtonPressed()
{
	if (!CombatComponent || bDisabledGameplay)
	{
		return;
	}

	if (CombatComponent->bHoldingFlag)
	{
		return;
	}
	
	if (IsAiming())
	{
		CombatComponent->SetAiming(false);
	}
	else
	{
		CombatComponent->SetAiming(true);
	}
}

void APartyCharacter::FireButtonPressed()
{	
	if (CombatComponent == nullptr || bDisabledGameplay)
	{
		return;
	}

	if (CombatComponent->bHoldingFlag)
	{
		return;
	}
	
	CombatComponent->FireButtonPressed(true);
}

void APartyCharacter::FireButtonReleased()
{	
	if (CombatComponent == nullptr || bDisabledGameplay)
	{
		return;
	}
	
	if (CombatComponent && CombatComponent->bHoldingFlag)
	{
		return;
	}
	
	CombatComponent->FireButtonPressed(false);
}

void APartyCharacter::ReloadButtonPressed()
{
	if (bDisabledGameplay)
	{
		return;
	}
	
	if (CombatComponent && CombatComponent->bHoldingFlag)
	{
		return;
	}
	
	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}

void APartyCharacter::Jump()
{
	if (bDisabledGameplay)
	{
		return;
	}

	if (CombatComponent && CombatComponent->bHoldingFlag)
	{
		return;
	}
	
	Super::Jump();
}

void APartyCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2d InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);

		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

float APartyCharacter::CalculateSpeed() const
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	return Velocity.Size();
}

void APartyCharacter::Destroyed()
{
	Super::Destroyed();

	const bool bMatchNotInProgress = PartyGameMode && PartyGameMode->GetMatchState() != MatchState::InProgress;
	
	if (CombatComponent && CombatComponent->EquippedWeapon && bMatchNotInProgress)
	{
		CombatComponent->EquippedWeapon->Destroy();
	}
}

void APartyCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) return;

	const float Speed = CalculateSpeed();

	const bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		const FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		const FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}

	else if (Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		bUseControllerRotationYaw = true;
		AO_Yaw = 0.f;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning; 
	}

	CalculateAO_Pitch();
}

void APartyCharacter::SimProxisTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr )
	{
		return;
	}

	bRotateRootBone = false;
	if (CalculateSpeed() > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	ProxiRotatorLastFrame = ProxiRotation;
	ProxiRotation = GetActorRotation();
	ProxiYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxiRotation, ProxiRotatorLastFrame).Yaw;

	if (FMath::Abs(ProxiYaw) > TurnThreshold)
	{
		TurningInPlace = ProxiYaw > TurnThreshold ? ETurningInPlace::ETIP_Right : ETurningInPlace::ETIP_Left;
	}
	else
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
}

bool APartyCharacter::IsHoldingTheFlag() const
{
	if (CombatComponent == nullptr)
	{
		return false;
	}
	
	return CombatComponent->bHoldingFlag;
}

void APartyCharacter::SetHoldingTheFlag(bool bIsHolding) const
{
	if (CombatComponent == nullptr)
	{
		return;
	}

	CombatComponent->bHoldingFlag = bIsHolding;
}

void APartyCharacter::ApplyHealthDamageEffect(const float DamageToHealth) const
{
	if (IsValid(DamageEffect))
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageEffect.Get(), 1.0f, EffectContext);

		if (SpecHandle.IsValid())
		{
			FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
			Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("GameplayEffect.Damage")), DamageToHealth * (-1));

			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec);
		}
	}
}

void APartyCharacter::ApplyShieldDamageEffect(const float DamageToShield) const
{
	if (IsValid(DamageEffect))
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(ShieldDamageEffect.Get(), 1.0f, EffectContext);

		if (SpecHandle.IsValid())
		{
			FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
			Spec->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("GameplayEffect.ShieldDamage")), DamageToShield * (-1));

			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec);
		}
	}
}

void APartyCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                    AController* InstigatorController, AActor* DamageCauser)
{
	if (PartyGameMode == nullptr)
	{
		return;
	}

	Damage = PartyGameMode->CalculateDamage(InstigatorController, Controller, Damage);

	const float DamageToHealth = FMath::Clamp(Damage - AttributeSet->GetShield(), 0.0f, AttributeSet->GetMaxHealth());
	ApplyShieldDamageEffect(Damage);
	ApplyHealthDamageEffect(DamageToHealth);

	float CurrentHealth = AttributeSet->GetHealth();
	
	UpdateHUDHealth();
	UpdateHUDShield();

	
	
	if (CurrentHealth > 0.f)
	{
		PlayHitReactionMontage();
	}
	else
	{
		if (Controller)
		{
			if (PartyGameMode)
			{
				APartyPlayerController* AttackerController = Cast<APartyPlayerController>(InstigatorController);
				PartyGameMode->PlayerEliminated(this, CachedPartyPlayerController, AttackerController);
			}
		}
	}
}

void APartyCharacter::SetupCollisionBoxes()
{
	/**
	* Abomination for server rewind stupidity
	*/

	HeadBox = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	HeadBox->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), HeadBox);

	PelvisBox = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	PelvisBox->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), PelvisBox);

	Spine_02Box = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	Spine_02Box->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), Spine_02Box);

	Spine_03Box = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	Spine_03Box->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), Spine_03Box);

	UpperArm_1Box = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	UpperArm_1Box->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), UpperArm_1Box);

	UpperArm_2Box = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	UpperArm_2Box->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), UpperArm_2Box);

	LowerArm_1Box = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	LowerArm_1Box->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), LowerArm_1Box);

	LowerArm_2Box = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	LowerArm_2Box->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), LowerArm_2Box);

	Hand_1Box = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	Hand_1Box->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), Hand_1Box);

	Hand_2Box = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	Hand_2Box->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), Hand_2Box);
	
	BackpackBox = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	BackpackBox->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), BackpackBox);

	BlanketBox = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket_l"));
	BlanketBox->SetupAttachment(GetMesh(), FName("blanket_l"));
	HitCollisionBoxes.Add(FName("blanket_l"), BlanketBox);

	UpperLeg_1Box = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	UpperLeg_1Box->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), UpperLeg_1Box);

	UpperLeg_2Box = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	UpperLeg_2Box->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), UpperLeg_2Box);

	LowerLeg_1Box = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	LowerLeg_1Box->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), LowerLeg_1Box);

	LowerLeg_2Box = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	LowerLeg_2Box->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), LowerLeg_2Box);

	Foot_1Box = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	Foot_1Box->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), Foot_1Box);

	Foot_2Box = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	Foot_2Box->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), HeadBox);

	for (const auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}
