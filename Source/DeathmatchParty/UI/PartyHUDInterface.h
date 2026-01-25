// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PartyHUDInterface.generated.h"


UINTERFACE(MinimalAPI)
class UPartyHUDInterface : public UInterface
{
	GENERATED_BODY()
};


class DEATHMATCHPARTY_API IPartyHUDInterface
{
	GENERATED_BODY()

public:

	virtual void SetHealth(float Current, float Max) = 0;
	virtual void SetShield(float Current, float Max) = 0;
	virtual void SetMatchTime(float Seconds) = 0;
	
};
