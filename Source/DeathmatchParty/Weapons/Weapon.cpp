#include "Weapons/Weapon.h"

#include "Characters/PartyCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/PartyPlayerController.h"
#include "Weapons/WeaponTypes.h"
#include "Weapons/BulletShell.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	
	//WeaponMesh->SetupAttachment(RootComponent);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));

	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::EnableCustomDepth(bool bEnable) const
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	SetReplicateMovement(true);
	
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	ResolveOwnerReferences();

	if (Owner == nullptr)
	{
		PartyControllerOwner = nullptr;
		PartyCharacterOwner = nullptr;
	}
	else
	{
		ResolveOwnerReferences();
		
		if (PartyCharacterOwner && PartyCharacterOwner->GetEquippedWeapon() && PartyCharacterOwner->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	
	SetOwner(nullptr);

	PartyCharacterOwner = nullptr;
	PartyControllerOwner = nullptr;
}

bool AWeapon::IsEmpty()
{
	return WeaponAmmo <= 0;
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	OnWeaponStateSet();
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

void AWeapon::OnRep_WeaponAmmo()
{
	SetHUDAmmo();
}

void AWeapon::OnWeaponStateSet()
{
	switch(WeaponState)
	{
	case EWeaponState::EWS_Initial:
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		break;
	case (EWeaponState::EWS_Equipped):
		OnEquipped();
		OnWeaponPickedUp.Broadcast();
		break;
	case (EWeaponState::EWS_Dropped):
		OnDropped();
		break;
	case (EWeaponState::EWS_Backup):
		OnBackup();
		OnWeaponPickedUp.Broadcast();
		break;
	default:
		break;
	}
}

void AWeapon::OnEquipped()
{
	DisplayPickupWidget(false);
	
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ResolveOwnerReferences();

	if (HasAuthority() && bUseServerSideRewind)
	{
		if (PartyControllerOwner && !PartyControllerOwner->HighPingDelegate.IsBound())
		{
			PartyControllerOwner->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	if (HasAuthority() && bUseServerSideRewind)
	{
		if (PartyControllerOwner && PartyControllerOwner->HighPingDelegate.IsBound())
		{
			PartyControllerOwner->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnBackup()
{
	DisplayPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (HasAuthority() && bUseServerSideRewind)
	{
		if (PartyControllerOwner && PartyControllerOwner->HighPingDelegate.IsBound())
		{
			PartyControllerOwner->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	WeaponAmmo = FMath::Clamp(WeaponAmmo + AmmoToAdd, 0, MagazineCapacity);
	SetHUDAmmo();
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimationAsset)
	{
		WeaponMesh->PlayAnimation(FireAnimationAsset, false);
	}
	
	SpendRound();

	if (BulletShell)
	{
		UWorld* World = GetWorld();
		if (WeaponMesh == nullptr || World == nullptr)
		{
			return;
		}

		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket == nullptr)
		{
			return;
		}

		const FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
		
		World->SpawnActor<ABulletShell>(
			BulletShell,
			SocketTransform.GetLocation(),
			SocketTransform.GetRotation().Rotator()
		);
	}
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority())
	{
		return;
	}
	
	WeaponAmmo = FMath::Clamp(WeaponAmmo + AmmoToAdd, 0, MagazineCapacity);
	SetHUDAmmo();
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APartyCharacter* OtherPartyCharacter = Cast<APartyCharacter>(OtherActor))
	{
		if (WeaponType == EWeaponType::EWT_Flag && OtherPartyCharacter->GetTeam() == WeaponTeam)
		{
			return;
		}

		if (OtherPartyCharacter->IsHoldingTheFlag())
		{
			return;
		}
		
		OtherPartyCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	if (APartyCharacter* OtherPartyCharacter = Cast<APartyCharacter>(OtherActor))
	{
		if (WeaponType == EWeaponType::EWT_Flag && OtherPartyCharacter->GetTeam() == WeaponTeam)
		{
			return;
		}
		
		if (OtherPartyCharacter->IsHoldingTheFlag())
		{
			return;
		}
		
		OtherPartyCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetHUDAmmo()
{
	if (PartyControllerOwner)
	{
		PartyControllerOwner->SetHUDWeaponAmmo(WeaponAmmo);
	}
}

FVector_NetQuantize AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;
	
	return FVector_NetQuantize(TraceStart + ToEndLoc * TRACE_LENGHT / ToEndLoc.Size());
}

void AWeapon::MultipleTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets, uint32 NumberOfShots)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for (uint32 i = 0; i < NumberOfShots; ++i)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGHT / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	}
}

void AWeapon::SpendRound()
{
	WeaponAmmo = FMath::Clamp(WeaponAmmo - 1, 0, MagazineCapacity);
	SetHUDAmmo();
}

void AWeapon::DisplayPickupWidget(bool bDisplayWidget) const
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bDisplayWidget);
	}
}

void AWeapon::ResolveOwnerReferences()
{
	PartyCharacterOwner = Cast<APartyCharacter>(GetOwner());

	PartyControllerOwner = PartyCharacterOwner
		? Cast<APartyPlayerController>(PartyCharacterOwner->Controller)
		: nullptr;
}
