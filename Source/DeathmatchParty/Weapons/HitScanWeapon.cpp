// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScanWeapon.h"

#include "Characters/PartyCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameMechanics/LagCompensationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "PlayerController/PartyPlayerController.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}
	
	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleSocket == nullptr)
	{
		return;
	}
	
	FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
	FVector Start = SocketTransform.GetLocation();

	FHitResult FireHit;
	WeaponTraceHit(Start, HitTarget, FireHit);

	if (FireHit.bBlockingHit)
	{
		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (OwnerPawn == nullptr)
		{
			return;
		}
		
		AController* OwnerController = OwnerPawn->GetController();
		APartyCharacter* HitCharacter = Cast<APartyCharacter>(FireHit.GetActor());
		
		if (HitCharacter && OwnerController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{

				bool bIsHeadshot = FireHit.BoneName.ToString() == FString("head");
				const float DamageToApply = bIsHeadshot ? HeadshotDamage : Damage;
				
				UGameplayStatics::ApplyDamage(
					HitCharacter,
					DamageToApply,
					OwnerController,
					this,
					UDamageType::StaticClass()
				);
			}

			if (!HasAuthority() && bUseServerSideRewind)
			{
				PartyCharacterOwner = PartyCharacterOwner == nullptr ? Cast<APartyCharacter>(OwnerPawn) : PartyCharacterOwner;
				PartyControllerOwner = PartyControllerOwner == nullptr ? Cast<APartyPlayerController>(OwnerController) : PartyControllerOwner;

				if (PartyControllerOwner && PartyCharacterOwner && PartyCharacterOwner->GetLagCompensationComponent() && PartyCharacterOwner->IsLocallyControlled())
				{
					PartyCharacterOwner->GetLagCompensationComponent()->ServerScoreRequest(
						HitCharacter,
						Start,
						HitTarget,
						PartyControllerOwner->GetServerTime() - PartyControllerOwner->SingleTripTime
					);
				}
			}
		}

		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				World,
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
				FireHit.ImpactNormal.Rotation()
				);
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	if (UWorld* World = GetWorld())
	{
		const FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
		
		World->LineTraceSingleByChannel(
			OutHit, TraceStart, End, ECollisionChannel::ECC_Visibility);
		
		FVector BeamEnd = End;
		
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		
		if (BeamParticles)
		{
			if (UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true))
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
