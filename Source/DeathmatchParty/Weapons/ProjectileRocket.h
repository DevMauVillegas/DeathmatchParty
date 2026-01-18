// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Projectile.h"
#include "ProjectileRocket.generated.h"

class URocketMovementComponent;

UCLASS()
class DEATHMATCHPARTY_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditAnywhere, Category="Projectile Settings")
	USoundAttenuation* LoopingSoundAttenuation;
	
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	
public:
	AProjectileRocket();

	UPROPERTY(VisibleAnywhere)
	URocketMovementComponent* RocketMovementComponent;
};
