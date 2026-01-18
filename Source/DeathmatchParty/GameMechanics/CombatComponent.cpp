#include "GameMechanics/CombatComponent.h"

#include "Camera/CameraComponent.h"
#include "Characters/PartyCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/PartyPlayerController.h"
#include "Weapons/Weapon.h"
#include "Weapons/WeaponTypes.h"
#include "Sound/SoundCue.h"
#include "Weapons/Shotgun.h"

UCombatComponent::UCombatComponent(): bFireButtonPressed(false), CrosshairVelocityFactor(0), DefaultFOV(0),
                                      CurrentFOV(0), BaseSpread(0),
                                      HUDPackage(),
                                      PartyCharacter(nullptr), PC(nullptr),
                                      HUD(nullptr),
                                      FollowCamera(nullptr),
                                      EquippedWeapon(nullptr),
                                      BackupWeapon(nullptr),
                                      CarriedAmmo(0)
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 550.f;
	AimWalkSpeed = 375.f;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PartyCharacter)
	{
		PartyCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		FollowCamera = PartyCharacter->GetFollowCamera();
		
		if (FollowCamera)
		{
			DefaultFOV = FollowCamera->FieldOfView;
			CurrentFOV = DefaultFOV;
		}

		if (PartyCharacter->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PartyCharacter && PartyCharacter->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		SetHUDCrosshair(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr || FollowCamera == nullptr)
	{
		return;
	}

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(
			CurrentFOV,
			EquippedWeapon->GetZoomedFOV(),
			DeltaTime,
			EquippedWeapon->GetZoomSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(
			CurrentFOV,
			DefaultFOV,
			DeltaTime,
			ZoomInterpSpeed);
	}

	FollowCamera->SetFieldOfView(CurrentFOV);
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr)
	{
		return false;
	}

	const bool bIsShotgun = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun;
	const bool bIsReloading = CombatState == ECombatState::ECS_Reloading;
	const bool bNoEmptyAndCanFire = !EquippedWeapon->IsEmpty() && bCanFire;

	// To allow the shotgun to fire while reloading
	if (bNoEmptyAndCanFire && bIsShotgun && bIsReloading)
	{
		return true;
	}
	
	return bNoEmptyAndCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingRifleAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Sniper, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GranadeLauncher, StartingGranadeAmmo);
}

void UCombatComponent::PickUpAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] += AmmoAmount;
		UpdateCarriedAmmo();
	}

	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (PartyCharacter == nullptr)
	{
		return;
	}
	
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);

	PartyCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;

	if (PartyCharacter->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper)
	{
		PartyCharacter->ShowSniperScopeWidget(bIsAiming);
	}
	
	if (PartyCharacter->IsLocallyControlled()) bAimButtonPressed = bIsAiming;
}

void UCombatComponent::OnRep_Aiming()
{
	if (PartyCharacter && PartyCharacter->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, BackupWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, bHoldingFlag);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;

	if (PartyCharacter)
	{
		PartyCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && PartyCharacter)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		AttachPrimaryWeapon(EquippedWeapon);

		PartyCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		PartyCharacter->bUseControllerRotationYaw = true;

		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->EnableCustomDepth(false);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_BackupWeapon()
{
	if (BackupWeapon && PartyCharacter)
	{
		BackupWeapon->SetWeaponState(EWeaponState::EWS_Backup);

		AttachBackupWeapon(BackupWeapon);

		PlayEquipWeaponSound(BackupWeapon);

		BackupWeapon->EnableCustomDepth(true);
	}
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (PartyCharacter && PartyCharacter->IsLocallyControlled() && !PartyCharacter->HasAuthority()) return;
	ShotgunLocalFire(TraceHitTargets);
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		FVector CrosshairWorldPosition;
		FVector CrosshairWorldDirection;
		
		bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
			UGameplayStatics::GetPlayerController(this, 0),
			CrosshairLocation,
			CrosshairWorldPosition,
			CrosshairWorldDirection
			);

		if (bScreenToWorld)
		{
			FVector Start = CrosshairWorldPosition;

			if (PartyCharacter)
			{
				float DistanceToCharacter = (PartyCharacter->GetActorLocation() - Start).Size();
				Start += CrosshairWorldDirection * (DistanceToCharacter + 75.f);
			}
			
			FVector End = Start + CrosshairWorldDirection * TRACE_LENGHT;
			
			GetWorld()->LineTraceSingleByChannel(
				TraceHitResult,
				Start,
				End,
				ECC_Visibility);
			
			if (!TraceHitResult.bBlockingHit)
			{
				TraceHitResult.ImpactPoint = End;
			}
			
			if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>())
			{
				HUDPackage.CrosshairColor = FLinearColor::Red;
			}
			else
			{
				HUDPackage.CrosshairColor = FLinearColor::White;
			}
		}
	}
}

void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
	if (PartyCharacter == nullptr || PartyCharacter->Controller == nullptr)
	{
		return;
	}

	PC = PC == nullptr ? Cast<APartyPlayerController>(PartyCharacter->Controller) : PC;

	if (PC == nullptr)
	{
		return;
	}
	
	HUD = HUD == nullptr ? Cast<APartyHUD>(PC->GetHUD()) : HUD;

	if (HUD == nullptr)
	{
		return;
	}

	if (EquippedWeapon)
	{
		HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter;
		HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
		HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom;
		HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
		HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;
	}
	else
	{
		HUDPackage.CrosshairCenter = nullptr;
		HUDPackage.CrosshairTop = nullptr;
		HUDPackage.CrosshairBottom = nullptr;
		HUDPackage.CrosshairLeft = nullptr;
		HUDPackage.CrosshairRight = nullptr;
	}

	FVector2D WalkSpeedRange(0.f, PartyCharacter->GetCharacterMovement()->MaxWalkSpeed);
	FVector2D VelocityMultiplierRange(0.f, 1.f);
	FVector Velocity = PartyCharacter->GetVelocity();
	Velocity.Z = 0.f;
	
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(\
		WalkSpeedRange,
		VelocityMultiplierRange,
		Velocity.Size());
	
	if (PartyCharacter->GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.5f, DeltaTime, 2.5);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 10.f);
	}

	if (bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, -0.5f, DeltaTime, 30.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);
	
	HUDPackage.CrosshairSpread =
		EquippedWeapon ? EquippedWeapon->GetAimSpread() : 0.5f +
			CrosshairShootingFactor +
				CrosshairVelocityFactor +
					CrosshairInAirFactor +
						CrosshairAimFactor;
	
	HUD->SetHUFPackage(HUDPackage);
	
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (PartyCharacter && PartyCharacter->IsLocallyControlled()) return;
	
	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	MulticastFire(TraceHitTarget);
	
	if (PartyCharacter->HasAuthority() && CombatState == ECombatState::ECS_Reloading)
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if (EquippedWeapon)
	{
		return FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
	}

	return true;
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	if (EquippedWeapon)
	{
		return FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
	}

	return true;
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bLocallyReloading = false;
		bCanFire = false;
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);

		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.75;

			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile:
				FireProjectileWeapon(HitResult.ImpactPoint);
				break;
			case EFireType::EFT_HitScan:
				FireHitScanWeapon(HitResult.ImpactPoint);
				break;
			case EFireType::EFT_MultipleHitScan:
				FireMultipleHitScanWeapon(HitResult.ImpactPoint);
				break;
			default:
				break;
			}
		}
		
		StartFireTimer();
	}
}

void UCombatComponent::FireProjectileWeapon(const FVector_NetQuantize& TraceHitTarget)
{
	LocalFire(TraceHitTarget);
	ServerFire(TraceHitTarget, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireHitScanWeapon(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon)
	{
		const FVector_NetQuantize HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(TraceHitTarget) : TraceHitTarget;
		LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireMultipleHitScanWeapon(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon)
	{
		TArray<FVector_NetQuantize> HitTargets;
		const uint32 NumberOfPellets = Cast<AShotgun>(EquippedWeapon)->NumberOfPellets;
		EquippedWeapon->MultipleTraceEndWithScatter(TraceHitTarget, HitTargets, NumberOfPellets);
		
		ShotgunLocalFire(HitTargets);
		ServerShotgunFire(HitTargets, EquippedWeapon->FireDelay);
		
	}
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	
	if (PartyCharacter)
	{
		PartyCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun == nullptr || PartyCharacter == nullptr) return;
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		PartyCharacter->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		Fire();
	}
}

int32 UCombatComponent::GetCarriedAmmo()
{
	return CarriedAmmo;
}

int32 UCombatComponent::GetCurrentWeaponAmmo()
{
	return EquippedWeapon->GetAmmo();
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	if (PC)
	{
		PC->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || PartyCharacter == nullptr)
	{
		return;
	}
	
	PartyCharacter->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&ThisClass::FireTimerFinished,
		EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
}

void UCombatComponent::EquipWeapon(AWeapon* inWeapon)
{
	if (PartyCharacter == nullptr || inWeapon == nullptr)
	{
		return;
	}
	
	if (inWeapon->GetWeaponType() == EWeaponType::EWT_Flag)
	{
		PartyCharacter->Crouch();
		//PartyCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
		//PartyCharacter->bUseControllerRotationYaw = false;
		bHoldingFlag = true;
		inWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachFlagToLeftHand(inWeapon);
		inWeapon->SetOwner(PartyCharacter);
		TheFlag = inWeapon;
	}
	else
	{
		if (BackupWeapon == nullptr && EquippedWeapon != nullptr)
		{
			EquipBackupWeapon(inWeapon);
		}
		else
		{
			bHoldingFlag = false;
			EquipPrimaryWeapon(inWeapon);
		}
		
		PartyCharacter->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* inWeapon)
{
	if (inWeapon->EquipSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, inWeapon->EquipSound, PartyCharacter->GetActorLocation());
	}
}

void UCombatComponent::SwapWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}

	PartyCharacter->PlaySwapMontage();
	CombatState = ECombatState::ECS_SwappingWeapon;
	
	AWeapon* WeaponHolder = EquippedWeapon;
	EquippedWeapon = BackupWeapon;
	BackupWeapon = WeaponHolder;
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* inWeapon)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
	
	EquippedWeapon = inWeapon;
	
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	AttachPrimaryWeapon(EquippedWeapon);

	EquippedWeapon->SetOwner(PartyCharacter);
	EquippedWeapon->SetHUDAmmo();

	EWeaponType WeaponType = EquippedWeapon->GetWeaponType();
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmo = CarriedAmmoMap[WeaponType];
	}
	
	if (PC)
	{
		PC->SetHUDCarriedAmmo(CarriedAmmo);
	}

	PlayEquipWeaponSound(EquippedWeapon);

	EquippedWeapon->EnableCustomDepth(false);
}

void UCombatComponent::AttachPrimaryWeapon(AWeapon* inWeapon) const
{
	if (PartyCharacter == nullptr || PartyCharacter->GetMesh() == nullptr || inWeapon == nullptr)
	{
		return;
	}
	
	if (const USkeletalMeshSocket* HandSocket = PartyCharacter->GetMesh()->GetSocketByName(FName("HandSocket")))
	{
		HandSocket->AttachActor(inWeapon, PartyCharacter->GetMesh());
	}
}

void UCombatComponent::EquipBackupWeapon(AWeapon* inWeapon)
{
	if (inWeapon == nullptr)
	{
		return;
	}
	BackupWeapon = inWeapon;
	
	BackupWeapon->SetWeaponState(EWeaponState::EWS_Backup);
	
	AttachBackupWeapon(BackupWeapon);

	BackupWeapon->SetOwner(PartyCharacter);
	PlayEquipWeaponSound(BackupWeapon);

	BackupWeapon->EnableCustomDepth(true);
}

void UCombatComponent::AttachBackupWeapon(AWeapon* inWeapon) const
{
	if (PartyCharacter == nullptr || PartyCharacter->GetMesh() == nullptr || inWeapon == nullptr)
	{
		return;
	}

	if (const USkeletalMeshSocket* BackupSocket = PartyCharacter->GetMesh()->GetSocketByName(FName("BackupSocket")))
	{
		BackupSocket->AttachActor(inWeapon, PartyCharacter->GetMesh());
	}
}

void UCombatComponent::AttachFlagToLeftHand(AWeapon* inWeapon) const
{
	if (PartyCharacter == nullptr || PartyCharacter->GetMesh() == nullptr || inWeapon == nullptr)
	{
		return;
	}

	if (const USkeletalMeshSocket* BackupSocket = PartyCharacter->GetMesh()->GetSocketByName(FName("FlagSocket")))
	{
		BackupSocket->AttachActor(inWeapon, PartyCharacter->GetMesh());
	}
}

bool UCombatComponent::ShouldSwapWeapons() const
{
	return (EquippedWeapon != nullptr && BackupWeapon != nullptr);
}


/*
 * RELOAD SECTION
 */

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr)
	{
		return 0;
	}

	int32 RoomInMagazine = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();
	EWeaponType WeaponType = EquippedWeapon->GetWeaponType();
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		int32 AmountCarried = CarriedAmmoMap[WeaponType];
		int32 Least = FMath::Min(AmountCarried, RoomInMagazine);
		return FMath::Clamp(RoomInMagazine, 0, Least);
	}

	return 0;
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		if (PartyCharacter && !PartyCharacter->IsLocallyControlled())
		{
			HandleReload();
		}
		break;
	case ECombatState::ECS_SwappingWeapon:
		if (PartyCharacter && !PartyCharacter->IsLocallyControlled())
		{
			PartyCharacter->PlaySwapMontage();
		}
		break;
	default:
		break;
	}
}

void UCombatComponent::OnRep_HoldingTheFlag()
{
	if (bHoldingFlag && PartyCharacter && PartyCharacter->IsLocallyControlled())
	{
		PartyCharacter->Crouch();
	}
}

void UCombatComponent::HandleReload()
{
	if (PartyCharacter)
	{
		PartyCharacter->PlayReloadMontage();
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	PC = PC == nullptr ? Cast<APartyPlayerController>(PartyCharacter->Controller) : PC;
	if (PC)
	{
		PC->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}
	
	int32 ReloadAmount = AmountToReload();
	EWeaponType WeaponType = EquippedWeapon->GetWeaponType();
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[WeaponType];
		EquippedWeapon->AddAmmo(ReloadAmount);
	}

	if (PC)
	{
		PC->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (PartyCharacter == nullptr || EquippedWeapon == nullptr || EquippedWeapon->GetMagazineCapacity() == EquippedWeapon->GetAmmo())
	{
		return;
	}

	CombatState = ECombatState::ECS_Reloading;
	
	if (!PartyCharacter->IsLocallyControlled())
	{
		HandleReload();
	}
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading && !bLocallyReloading)
	{
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
	}
}

void UCombatComponent::FinishReloading()
{
	if (PartyCharacter == nullptr)
	{
		return;
	}

	if (PartyCharacter->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
		bLocallyReloading = false;
	}
}

void UCombatComponent::FinishSwapping()
{
	if (PartyCharacter && PartyCharacter->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::FinishSwapAttach()
{
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachPrimaryWeapon(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();

	EWeaponType WeaponType = EquippedWeapon->GetWeaponType();
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmo = CarriedAmmoMap[WeaponType];
	}
	
	if (PC)
	{
		PC->SetHUDCarriedAmmo(CarriedAmmo);
	}

	PlayEquipWeaponSound(EquippedWeapon);

	BackupWeapon->SetWeaponState(EWeaponState::EWS_Backup);
	
	AttachBackupWeapon(BackupWeapon);
}
