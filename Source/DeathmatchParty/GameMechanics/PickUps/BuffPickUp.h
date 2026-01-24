// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameMechanics/PickUps/PickUp.h"
#include "BuffPickUp.generated.h"

struct FGameplayTag;
class UGameplayEffect;
/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API ABuffPickUp : public APickUp
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="PickUpSettings")
	float EffectAmount;

	UPROPERTY(EditDefaultsOnly, Category="PickUpSettings")
	float EffectDuration;
	
	UPROPERTY(EditDefaultsOnly, Category="PickUpSettings")
	float EffectPeriod;

	UPROPERTY(EditDefaultsOnly, Category="PickUpSettings")
	FGameplayTag EffectTag;
	
public:
	
	ABuffPickUp();

	UPROPERTY(EditAnywhere, Category="PickUpSettings")
	TSubclassOf<UGameplayEffect> GameplayEffect;

	virtual void Destroyed() override;

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
