// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ProjectileWeapon.h"
#include "Weapons/Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	UWorld* World = GetWorld();
	const USkeletalMeshComponent* FiringWeaponMesh = GetWeaponMesh();
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	
	if (ProjectileClass == nullptr || World == nullptr || FiringWeaponMesh == nullptr || InstigatorPawn == nullptr)
	{
		return;
	}
	
	const USkeletalMeshSocket* MuzzleFlashSocket = FiringWeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket == nullptr)
	{
		return;
	}
	
	SpawnProjectileAfterFiring(
		HitTarget,
		World,
		FiringWeaponMesh,
		InstigatorPawn,
		MuzzleFlashSocket
	);
}

void AProjectileWeapon::SpawnProjectileAfterFiring(const FVector& HitTarget, UWorld* World, const USkeletalMeshComponent* FiringWeaponMesh, APawn* InstigatorPawn, const USkeletalMeshSocket* MuzzleFlashSocket)
{
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(FiringWeaponMesh);
	const FVector ToTarget = HitTarget - SocketTransform.GetLocation();
	const FRotator TargetRotation = ToTarget.Rotation();
	
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = InstigatorPawn;

	AProjectile* SpawnedProjectile = nullptr;
	
	if (bUseServerSideRewind)
	{
		if (InstigatorPawn->HasAuthority())
		{
			const TSubclassOf<AProjectile> ProjectileToUse = InstigatorPawn->IsLocallyControlled() ? ProjectileClass : ServerSideRewindProjectileClass;
			
			SpawnedProjectile = World->SpawnActor<AProjectile>(
				ProjectileToUse,
				SocketTransform.GetLocation(),
				TargetRotation,
				SpawnParameters
			);
			
			SpawnedProjectile->bUseServerSideRewind = false;

			if (InstigatorPawn->IsLocallyControlled())
			{
				SpawnedProjectile->Damage = Damage;
				SpawnedProjectile->HeadShotDamage = HeadshotDamage;
			}
		}
		else
		{
			SpawnedProjectile = World->SpawnActor<AProjectile>(
				ServerSideRewindProjectileClass,
				SocketTransform.GetLocation(),
				TargetRotation,
				SpawnParameters
			);
			
			if (InstigatorPawn->IsLocallyControlled())
			{
				SpawnedProjectile->bUseServerSideRewind = true;
				SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
				SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->ProjectileInitialSpeed;
				SpawnedProjectile->Damage = Damage;
				SpawnedProjectile->HeadShotDamage = HeadshotDamage;
			}
			else
			{
				SpawnedProjectile->bUseServerSideRewind = false;
			}
		}
	}
	else
	{
		if (InstigatorPawn->HasAuthority())
		{
			SpawnedProjectile = World->SpawnActor<AProjectile>(
				ProjectileClass,
				SocketTransform.GetLocation(),
				TargetRotation,
				SpawnParameters
			);
			
			SpawnedProjectile->bUseServerSideRewind = false;
			SpawnedProjectile->Damage = Damage;
			SpawnedProjectile->HeadShotDamage = HeadshotDamage;

		}
	}
}
