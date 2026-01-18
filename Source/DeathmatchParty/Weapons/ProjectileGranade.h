// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Projectile.h"
#include "ProjectileGranade.generated.h"

class USoundCue;

UCLASS()
class DEATHMATCHPARTY_API AProjectileGranade : public AProjectile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Projectile Settings")
	USoundCue* BounceSound;
	
public:
	AProjectileGranade();
	virtual void Destroyed() override;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
};
