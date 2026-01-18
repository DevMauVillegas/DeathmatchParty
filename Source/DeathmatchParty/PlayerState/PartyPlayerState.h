// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PartyTypes/Team.h"
#include "PartyPlayerState.generated.h"

class APartyPlayerController;
class APartyCharacter;
/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API APartyPlayerState : public APlayerState
{
	GENERATED_BODY()

	UPROPERTY()
	APartyCharacter* Character;

	UPROPERTY()
	APartyPlayerController* PC;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam PlayerTeam = ETeam::ET_NoTeam;

	UFUNCTION()
	void OnRep_Team();
	
public:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnRep_Defeats();
	virtual void OnRep_Score() override;
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatAmount);

	FORCEINLINE ETeam GetPlayerTeam() const { return PlayerTeam; }
	void SetPlayerTeam(ETeam NewPlayerTeam);
};
