#pragma once

#include "CoreMinimal.h"
#include "GameMechanics/PickUps/PickUp.h"
#include "SpeedPickUp.generated.h"

/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API ASpeedPickUp : public APickUp
{
	GENERATED_BODY()

	ASpeedPickUp();
	
	UPROPERTY(EditDefaultsOnly)
	float BaseSpeedBuff = 1500.0f;
	
	UPROPERTY(EditDefaultsOnly)
	float CrouchSpeedBuff = 800.0f;

	UPROPERTY(EditDefaultsOnly)
	float SpeedBuffTime = 15.0f;

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
