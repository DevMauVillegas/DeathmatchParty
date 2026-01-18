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

	UPROPERTY(EditAnywhere)
	float SpawnPickUpTimerMin;
	
	UPROPERTY(EditAnywhere)
	float SpawnPickUpTimerMax;
	
public:	
	// Sets default values for this actor's properties
	AWeaponSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SpawnPickUp();

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AWeapon>> PickUpClasses;

	UFUNCTION()
	void SpawnPickUpTimerFinished();

	UFUNCTION()
	void SpawnPickUpTimerStart();

	UPROPERTY()
	AWeapon* SpawnedPickUp;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
