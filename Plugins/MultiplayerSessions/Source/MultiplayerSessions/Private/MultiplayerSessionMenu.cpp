// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionMenu.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Components/Button.h"

void UMultiplayerSessionMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(100000);
	}
}

void UMultiplayerSessionMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMultiplayerSessionMenu::MenuTearDown()
{
	RemoveFromParent();
	if (const UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			const FInputModeGameOnly InputModeData;
			
			PC->SetInputMode(InputModeData);
			PC->SetShowMouseCursor(false);
		}
	}
}

void UMultiplayerSessionMenu::MenuSetup(int32 inNumPublicConnections, FString inMatchType, FString inLobbyPath)
{
	NumPublicConnections = inNumPublicConnections;
	MatchType = inMatchType;

	PathToLobby = inLobbyPath;
	
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);

	SetIsFocusable(true);

	if (UGameInstance* GI = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GI->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			
			PC->SetInputMode(InputModeData);
			PC->SetShowMouseCursor(true);
		}
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
		
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionComplete.AddUObject(this, &ThisClass::OnFindSessions);
	}
}

bool UMultiplayerSessionMenu::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	return true;
}

void UMultiplayerSessionMenu::NativeDestruct()
{
	MenuTearDown();
	
	Super::NativeDestruct();
}

void UMultiplayerSessionMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (UWorld* World = GetWorld())
		{
			World->ServerTravel(FString::Printf(TEXT("%s?listen"), *PathToLobby));
		}
	} else
	{
		HostButton->SetIsEnabled(true);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("ERROR CREATING THE SESSION"))
			);
		}
	}
}

void UMultiplayerSessionMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}
	
	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

		if (SettingsValue == MatchType)
		{
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}

	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMultiplayerSessionMenu::OnJoinSessions(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get();

	if (SubSystem)
	{
		IOnlineSessionPtr SessionInterface = SubSystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
			{
				PC->ClientTravel(Address, TRAVEL_Absolute);
			}
		}
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMultiplayerSessionMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMultiplayerSessionMenu::OnStartSession(bool bWasSuccessful)
{
}
