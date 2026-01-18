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

	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1500.0f;
	
	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 800.0f;

	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 15.0f;

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
};
