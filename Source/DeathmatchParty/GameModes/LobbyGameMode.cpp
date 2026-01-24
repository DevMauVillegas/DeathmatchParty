// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance == nullptr)
	{
		return;
	}

	const UMultiplayerSessionsSubsystem* MultiplayerSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	if (MultiplayerSubsystem == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}
	
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	
	// Use here the desired number of players set in the multiplayer sessions subsystem
	if (NumberOfPlayers == 2)
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
