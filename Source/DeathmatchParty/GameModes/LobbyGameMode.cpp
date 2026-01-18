// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UMultiplayerSessionsSubsystem* MultiplayerSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(MultiplayerSubsystem)
		
		// Use here the desired number of players set in the multiplayer sessions subsystem
		if (NumberOfPlayers == 2)
		{
			if (UWorld* World = GetWorld())
			{
				bUseSeamlessTravel = true;
				
				FString MatchType = MultiplayerSubsystem->DesiredMatchType;
				if (MatchType == "Deathmatch")
				{
					World->ServerTravel(FString("/Game/Assets/Maps/Asian_Village?listen"));
				}
					
				if (MatchType == "Teams")
				{
					World->ServerTravel(FString("/Game/Assets/Maps/Teams?listen"));
				}
					
				if (MatchType == "Flag")
				{
					World->ServerTravel(FString("/Game/Assets/Maps/Flag?listen"));
				}
			}
		}
	}
}
