#include "PartyPlayerState.h"

#include "Characters/PartyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/PartyPlayerController.h"


void APartyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APartyPlayerState, Defeats);
	DOREPLIFETIME(APartyPlayerState, PlayerTeam);
}

void APartyPlayerState::OnRep_Defeats()
{	
	Character = Character == nullptr ? Cast<APartyCharacter>(GetPawn()) : Character;

	if (Character)
	{
		PC = PC == nullptr ? Cast<APartyPlayerController>(Character->Controller) : PC;
		if (PC)
		{
			PC->SetHUDDefeats(Defeats);
		}
	}
}

void APartyPlayerState::AddToDefeats(int32 DefeatAmount)
{
	Defeats += DefeatAmount;
	
	Character = Character == nullptr ? Cast<APartyCharacter>(GetPawn()) : Character;

	if (Character)
	{
		PC = PC == nullptr ? Cast<APartyPlayerController>(Character->Controller) : PC;
		if (PC)
		{
			PC->SetHUDDefeats(Defeats);
		}
	}
}

void APartyPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<APartyCharacter>(GetPawn()) : Character;

	if (Character)
	{
		PC = PC == nullptr ? Cast<APartyPlayerController>(Character->Controller) : PC;
		if (PC)
		{
			PC->SetHUDScore(GetScore());
		}
	}
}

void APartyPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<APartyCharacter>(GetPawn()) : Character;

	if (Character)
	{
		PC = PC == nullptr ? Cast<APartyPlayerController>(Character->Controller) : PC;
		if (PC)
		{
			PC->SetHUDScore(GetScore());
		}
	}
}

void APartyPlayerState::OnRep_Team()
{
	if (APartyCharacter* PartyCharacter = Cast<APartyCharacter>(GetPawn()))
	{
		PartyCharacter->SetTeamColor(PlayerTeam);
	}
}

void APartyPlayerState::SetPlayerTeam(ETeam NewPlayerTeam)
{
	PlayerTeam = NewPlayerTeam;

	if (APartyCharacter* PartyCharacter = Cast<APartyCharacter>(GetPawn()))
	{
		PartyCharacter->SetTeamColor(PlayerTeam);
	}
}