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

	virtual void SetHealth(const float Current, const float Max) = 0;
	virtual void SetShield(const float Current, const float Max) = 0;
	virtual void SetScore(const int32 NewScore) = 0;

	virtual void HandleCooldown() = 0;	

	virtual void SetDefeats(const int32 NewDefeats) = 0;
	
	virtual void SetWeaponAmmo(const int32 NewWeaponAmmo) = 0;
	virtual void SetCarriedAmmo(const int32 NewCarriedAmmo) = 0;

	virtual void SetAnnouncementCountdown(const float NewAnnouncementCountdownTime) = 0;
	virtual void SetMatchTime(const float Seconds) = 0;
	
	virtual void SetRedTeamScore(const int32 NewRedTeamScore) = 0;
	virtual void SetBlueTeamScore(const int32 NewBlueTeamScore) = 0;
	virtual void HideTeamUIElements() = 0;

	virtual void DisplayConnectionWarning(const bool bDisplayConnectionWarning) = 0;
};
