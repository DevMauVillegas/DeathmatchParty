// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PartyGameMode.h"
#include "TeamPartyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API ATeamPartyGameMode : public APartyGameMode
{
	GENERATED_BODY()

protected:
	virtual void HandleMatchHasStarted() override;

public:
	ATeamPartyGameMode();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void PlayerEliminated(APartyCharacter* EliminatedCharacter, APartyPlayerController* VictimController, APartyPlayerController* AttackerController) override;

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
};
