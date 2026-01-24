// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PartyGameMode.generated.h"

class APartyCharacter;
class APartyPlayerController;
class APartyPlayerState;

namespace MatchState
{
	// Match duration ended. This will display winner and stats
	extern DEATHMATCHPARTY_API const FName Cooldown;	
}

UCLASS()
class DEATHMATCHPARTY_API APartyGameMode : public AGameMode
{
	GENERATED_BODY()

	float CountdownTime;

public:

	APartyGameMode();

	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime;
	
	UPROPERTY(EditDefaultsOnly)
	float MatchTime;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime;

	UPROPERTY(EditDefaultsOnly)
	float LevelStartingTime;

	bool bIsTeamMatch;
	
	virtual void PlayerEliminated(APartyCharacter* EliminatedCharacter, APartyPlayerController* VictimController, APartyPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

	void PlayerLeftGame(APartyPlayerState* LeavingPlayerState);

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
