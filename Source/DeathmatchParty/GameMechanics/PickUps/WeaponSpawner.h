// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponSpawner.generated.h"

class AWeapon;

UCLASS()
class DEATHMATCHPARTY_API AWeaponSpawner : public AActor
{
	GENERATED_BODY()
	
	FTimerHandle SpawnPickUpTimer;

	UPROPERTY(EditDefaultsOnly)
	float SpawnPickUpTimerMin;
	
	UPROPERTY(EditDefaultsOnly)
	float SpawnPickUpTimerMax;
	
public:
	
	AWeaponSpawner();

protected:

	virtual void BeginPlay() override;

	void SpawnPickUp();

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AWeapon>> PickUpClasses;

	UFUNCTION()
	void SpawnPickUpTimerFinished();

	UFUNCTION()
	void SpawnPickUpTimerStart();

	UPROPERTY(EditAnywhere)
	AWeapon* SpawnedPickUp;
};
