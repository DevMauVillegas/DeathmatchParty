// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMechanics/PickUps/PickUp.h"
#include "JumpPickUp.generated.h"

/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API AJumpPickUp : public APickUp
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float JumpVelocityBuff;

	UPROPERTY(EditDefaultsOnly)
	float JumpVelocityTime;

public:
	AJumpPickUp();

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
};
