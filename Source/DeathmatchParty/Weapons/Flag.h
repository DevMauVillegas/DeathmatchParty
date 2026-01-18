// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

class UStaticMeshComponent;
/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API AFlag : public AWeapon
{
	GENERATED_BODY()

private:

	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	UStaticMeshComponent* FlagMesh;

	UPROPERTY()
	FTransform InitialTransform;

public:
	
	AFlag();

	virtual void Dropped() override;
	
	void SetFlagVisibility(bool bIsVisible) const;

	FORCEINLINE FTransform GetInitialLocation() const { return InitialTransform; }

	void ResetFlag();
	
protected:
	virtual void BeginPlay() override;
	
	virtual void OnEquipped() override;
	virtual void OnDropped() override;
};
