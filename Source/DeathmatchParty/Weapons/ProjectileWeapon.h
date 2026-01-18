// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

class AProjectile;

/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ProjectileClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;
	
public:
	void SpawnProjectileAfterFiring(
		const FVector& HitTarget,
		UWorld* World,
		const USkeletalMeshComponent* FiringWeaponMesh,
		APawn* InstigatorPawn,
		const USkeletalMeshSocket* MuzzleFlashSocket);
	
	virtual void Fire(const FVector& HitTarget) override;
};
