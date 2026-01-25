// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/TeamPartyGameMode.h"

#include "GameState/PartyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/PartyPlayerController.h"
#include "PlayerState/PartyPlayerState.h"

void ATeamPartyGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	
	APartyGameState* PartyGameState = Cast<APartyGameState>(UGameplayStatics::GetGameState(this));
	if (PartyGameState == nullptr)
	{
		return;
	}
	
	for (TObjectPtr<APlayerState> PlayerState : PartyGameState->PlayerArray)
	{
		if (APartyPlayerState* PartyPlayerState = Cast<APartyPlayerState>(PlayerState.Get()); PartyPlayerState->GetPlayerTeam() == ETeam::ET_NoTeam)
		{
			const bool bIsBlueTeamBigger = PartyGameState->BlueTeam.Num() > PartyGameState->RedTeam.Num();
			bIsBlueTeamBigger ? PartyGameState->RedTeam.AddUnique(PartyPlayerState) : PartyGameState->BlueTeam.AddUnique(PartyPlayerState);
			PartyPlayerState->SetPlayerTeam(bIsBlueTeamBigger ? ETeam::ET_RedTeam : ETeam::ET_BlueTeam);
		}
	}
}

ATeamPartyGameMode::ATeamPartyGameMode()
{
	bIsTeamMatch = true;
}

void ATeamPartyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer == nullptr)
	{
		return;
	}

	APartyGameState* PartyGameState = Cast<APartyGameState>(UGameplayStatics::GetGameState(this));
	if (PartyGameState == nullptr)
	{
		return;
	}
	
	if (APartyPlayerState* PartyPlayerState = NewPlayer->GetPlayerState<APartyPlayerState>(); PartyPlayerState->GetPlayerTeam() == ETeam::ET_NoTeam)
	{
		const bool bIsBlueTeamBigger = PartyGameState->BlueTeam.Num() > PartyGameState->RedTeam.Num();
		bIsBlueTeamBigger ? PartyGameState->RedTeam.AddUnique(PartyPlayerState) : PartyGameState->BlueTeam.AddUnique(PartyPlayerState);
		PartyPlayerState->SetPlayerTeam(bIsBlueTeamBigger ? ETeam::ET_RedTeam : ETeam::ET_BlueTeam);
	}
}

void ATeamPartyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	if (Exiting == nullptr)
	{
		return;
	}
	
	APartyGameState* PartyGameState = Cast<APartyGameState>(UGameplayStatics::GetGameState(this));
	if (PartyGameState == nullptr)
	{
		return;
	}
	
	if (APartyPlayerState* PartyPlayerState = Exiting->GetPlayerState<APartyPlayerState>())
	{
		PartyGameState->RedTeam.Remove(PartyPlayerState);
		PartyGameState->BlueTeam.Remove(PartyPlayerState);
	}
}

void ATeamPartyGameMode::PlayerEliminated(APartyCharacter* EliminatedCharacter,
	APartyPlayerController* VictimController, APartyPlayerController* AttackerController)
{
	Super::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);

	APartyPlayerState* AttackerPlayerState = AttackerController != nullptr ? AttackerController->GetPlayerState<APartyPlayerState>() : nullptr;
	if (AttackerPlayerState == nullptr)
	{
		return;
	}

	APartyGameState* PartyGameState = Cast<APartyGameState>(UGameplayStatics::GetGameState(this));
	if (PartyGameState == nullptr)
	{
		return;
	}
	
	if (AttackerPlayerState->GetPlayerTeam() == ETeam::ET_BlueTeam)
	{
		PartyGameState->AddBlueTeamScore();
	}
	else if (AttackerPlayerState->GetPlayerTeam() == ETeam::ET_RedTeam)
	{
		PartyGameState->AddRedTeamScore();
	}
}

float ATeamPartyGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	if (Attacker == nullptr || Victim == nullptr)
	{
		return Super::CalculateDamage(Attacker, Victim, BaseDamage);
	}

	APartyPlayerState* AttackerPlayerState = Attacker->GetPlayerState<APartyPlayerState>();
	APartyPlayerState* VictimPlayerState = Victim->GetPlayerState<APartyPlayerState>();

	if (AttackerPlayerState == nullptr || VictimPlayerState == nullptr)
	{
		return Super::CalculateDamage(Attacker, Victim, BaseDamage);
	}

	float DamageToCause = BaseDamage;
	if (AttackerPlayerState->GetPlayerTeam() == VictimPlayerState->GetPlayerTeam())
	{
		DamageToCause = 0.0f;
	}
	
	return DamageToCause;
}
