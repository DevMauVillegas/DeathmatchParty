// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureFlagGameMode.h"
#include "GameMechanics/FlagZone.h"
#include "GameState/PartyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/Flag.h"

void ACaptureFlagGameMode::PlayerEliminated(APartyCharacter* EliminatedCharacter,
                                            APartyPlayerController* VictimController, APartyPlayerController* AttackerController)
{
	APartyGameMode::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);
}

void ACaptureFlagGameMode::FlagCaptured(const AFlag* Flag, const AFlagZone* FlagZone) const
{
	bool bValidCapture = Flag->GetWeaponTeam() != FlagZone->ZoneTeam;

	if (APartyGameState* PartyGameState = Cast<APartyGameState>(UGameplayStatics::GetGameState(this)))
	{
		switch (FlagZone->ZoneTeam)
		{
		case ETeam::ET_BlueTeam:
			PartyGameState->AddBlueTeamScore();
			break;
		case ETeam::ET_RedTeam:
			PartyGameState->AddRedTeamScore();
			break;
		case ETeam::ET_NoTeam:
		default:
			break;
		}
	}
}
