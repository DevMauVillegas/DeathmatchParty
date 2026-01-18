// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionMenu.generated.h"

class UButton;
class UMultiplayerSessionsSubsystem;

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionMenu : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void JoinButtonClicked();

	UFUNCTION()
	void HostButtonClicked();

	void MenuTearDown();

	// The subsystem designed to handle all online session functionality
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{4};

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString MatchType{TEXT("Deathmatch")};

	FString PathToLobby{TEXT("")};

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 inNumPublicConnections = 4, FString inMatchType = FString(TEXT("Deathmatch")), FString inLobbyPath = FString(TEXT("/Game/Maps/Lobby")));

protected:
	virtual bool Initialize() override;

	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSessions(EOnJoinSessionCompleteResult::Type Result);
	
};
