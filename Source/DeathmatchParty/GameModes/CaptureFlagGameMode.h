// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamPartyGameMode.h"
#include "CaptureFlagGameMode.generated.h"

class AFlag;
class AFlagZone;
/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API ACaptureFlagGameMode : public ATeamPartyGameMode
{
	GENERATED_BODY()

public:
	
	virtual void PlayerEliminated(APartyCharacter* EliminatedCharacter, APartyPlayerController* VictimController, APartyPlayerController* AttackerController) override;

	void FlagCaptured(AFlag* Flag, AFlagZone* FlagZone) const;
};
