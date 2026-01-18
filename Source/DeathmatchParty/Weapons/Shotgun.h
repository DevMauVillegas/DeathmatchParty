#pragma once

#include "CoreMinimal.h"
#include "Weapons/HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

	
public:

	void FireShotgun(TArray<FVector_NetQuantize> HitTargets);
	
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	uint32 NumberOfPellets = 10;
};
