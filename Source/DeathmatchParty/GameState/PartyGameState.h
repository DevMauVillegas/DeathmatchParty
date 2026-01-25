#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PartyGameState.generated.h"

class APartyPlayerState;

UCLASS()
class DEATHMATCHPARTY_API APartyGameState : public AGameState
{
	GENERATED_BODY()

	float TopScore = 0.0f;
	
public:

	UPROPERTY(Replicated)
	TArray<APartyPlayerState*> TopScoringPlayers;
	
	TArray<APartyPlayerState*> RedTeam;
	TArray<APartyPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.0f;


	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();

	void AddRedTeamScore();
	void AddBlueTeamScore();
	
	void UpdateTopScores(APartyPlayerState* ScoringPlayerState);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
