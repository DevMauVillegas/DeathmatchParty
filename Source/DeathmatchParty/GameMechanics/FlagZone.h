// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PartyTypes/Team.h"
#include "Weapons/Flag.h"
#include "FlagZone.generated.h"

class ACaptureFlagGameMode;
class UBoxComponent;

UCLASS()
class DEATHMATCHPARTY_API AFlagZone : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UBoxComponent* BoxComponent;

	UPROPERTY()
	ACaptureFlagGameMode* CaptureFlagGameMode;
	
public:
	AFlagZone();

	UPROPERTY(EditAnywhere)
	ETeam ZoneTeam;

protected:

	virtual void BeginPlay() override;
	void CapturingTheFlag(AFlag* OverlappingFlag);

	UFUNCTION()
	virtual void OnBoxOverlap(
		UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

};
