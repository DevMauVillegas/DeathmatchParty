// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUpSpawner.generated.h"

class APickUp;

UCLASS()
class DEATHMATCHPARTY_API APickUpSpawner : public AActor
{
	GENERATED_BODY()

	FTimerHandle SpawnPickUpTimer;

	UPROPERTY(EditAnywhere)
	float SpawnPickUpTimerMin;
	
	UPROPERTY(EditAnywhere)
	float SpawnPickUpTimerMax;
	
public:	
	APickUpSpawner();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	void SpawnPickUp();

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<APickUp>> PickUpClasses;

	UFUNCTION()
	void SpawnPickUpTimerFinished();

	UFUNCTION()
	void SpawnPickUpTimerStart(AActor* DestroyedActor);

	UPROPERTY()
	APickUp* SpawnedPickUp;
};
