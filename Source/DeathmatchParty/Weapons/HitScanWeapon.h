// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;

UCLASS()
class DEATHMATCHPARTY_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	
	virtual void Fire(const FVector& HitTarget) override;
	
protected:

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	USoundCue* FireSound;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	USoundCue* HitSound;
	
	void WeaponTraceHit(const FVector& Start, const FVector& End, FHitResult& OutHit);
};
