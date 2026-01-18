#include "Weapons/Shotgun.h"
#include "Sound/SoundCue.h"
#include "Characters/PartyCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameMechanics/LagCompensationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/PartyPlayerController.h"

void AShotgun::FireShotgun(TArray<FVector_NetQuantize> HitTargets)
{
	AWeapon::Fire(FVector());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr)
	{
		return;
	}
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector Start = SocketTransform.GetLocation();

	// Maps hit character to number of times hit
	TMap<APartyCharacter*, uint32> HitMap;
	TMap<APartyCharacter*, uint32> HeadshotHitMap;
	for (FVector_NetQuantize HitTarget : HitTargets)
	{
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		APartyCharacter* BlasterCharacter = Cast<APartyCharacter>(FireHit.GetActor());
		if (BlasterCharacter)
		{

			bool bIsHeadshot = FireHit.BoneName.ToString() == FString("head");
			const float DamageToApply = bIsHeadshot ? HeadshotDamage : Damage;

			if (bIsHeadshot)
			{
				HeadshotHitMap.Contains(BlasterCharacter) ? HeadshotHitMap[BlasterCharacter]++ : HeadshotHitMap.Emplace(BlasterCharacter, 1);
			}
			else
			{
				HitMap.Contains(BlasterCharacter) ? HitMap[BlasterCharacter]++ : HitMap.Emplace(BlasterCharacter, 1);
			}

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					.5f,
					FMath::FRandRange(-.5f, .5f)
				);
			}
		}
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	
	if (OwnerPawn == nullptr)
	{
		return;
	}

	AController* InstigatorController = OwnerPawn->GetController();

	if (InstigatorController == nullptr)
	{
		return;
	}
	
	TArray<APartyCharacter*> HitCharacterArray;
	TMap<APartyCharacter*, uint32> DamageMap;

	for (auto HitPair : HitMap)
	{
		if (HitPair.Key)
		{
			DamageMap.Emplace(HitPair.Key, HitPair.Value*Damage);
			HitCharacterArray.AddUnique(HitPair.Key);
		}
	}

	for (auto HeadHitPair : HeadshotHitMap)
	{
		if (HeadHitPair.Key)
		{
			DamageMap.Contains(HeadHitPair.Key) ? DamageMap[HeadHitPair.Key]+= HeadHitPair.Value*HeadshotDamage : DamageMap.Emplace(HeadHitPair.Key, HeadHitPair.Value*HeadshotDamage);
			HitCharacterArray.AddUnique(HeadHitPair.Key);
		}
	}

	for (auto DamagePair : DamageMap)
	{
		if (DamagePair.Key && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				UGameplayStatics::ApplyDamage(
					DamagePair.Key,
					DamagePair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
	
	if (!HasAuthority() && bUseServerSideRewind)
	{
		PartyCharacterOwner = PartyCharacterOwner == nullptr ? Cast<APartyCharacter>(OwnerPawn) : PartyCharacterOwner;
		PartyControllerOwner = PartyControllerOwner == nullptr ? Cast<APartyPlayerController>(InstigatorController) : PartyControllerOwner;

		if (PartyControllerOwner && PartyCharacterOwner && PartyCharacterOwner->GetLagCompensationComponent() && PartyCharacterOwner->IsLocallyControlled())
		{
			PartyCharacterOwner->GetLagCompensationComponent()->ShotgunServerScoreRequest(
				HitCharacterArray,
				Start,
				HitTargets,
				PartyControllerOwner->GetServerTime() - PartyControllerOwner->SingleTripTime
			);
		}
	}
	
}
