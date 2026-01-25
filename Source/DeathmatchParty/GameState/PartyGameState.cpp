#include "PartyGameState.h"
#include "Net/UnrealNetwork.h"
#include "DeathmatchParty/PlayerState/PartyPlayerState.h"
#include "PlayerController/PartyPlayerController.h"

void APartyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APartyGameState, TopScoringPlayers);
	DOREPLIFETIME(APartyGameState, RedTeamScore);
	DOREPLIFETIME(APartyGameState, BlueTeamScore);
}

void APartyGameState::OnRep_RedTeamScore()
{
	if (APartyPlayerController* PartyPlayerController = Cast<APartyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PartyPlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void APartyGameState::OnRep_BlueTeamScore()
{
	if (APartyPlayerController* PartyPlayerController = Cast<APartyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PartyPlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void APartyGameState::AddRedTeamScore()
{
	++RedTeamScore;

	if (APartyPlayerController* PartyPlayerController = Cast<APartyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PartyPlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void APartyGameState::AddBlueTeamScore()
{
	++BlueTeamScore;

	if (APartyPlayerController* PartyPlayerController = Cast<APartyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PartyPlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void APartyGameState::UpdateTopScores(APartyPlayerState* ScoringPlayerState)
{
	if (ScoringPlayerState == nullptr)
	{
		return;
	}

	const float PlayerScore = ScoringPlayerState->GetScore();
	
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayerState);
		TopScore = PlayerScore;
	}
	else if (PlayerScore == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayerState);
	}
	else if (PlayerScore > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayerState);
		TopScore = PlayerScore;
	}
}
