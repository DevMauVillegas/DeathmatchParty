// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PartyPlayerController.generated.h"

class APartyGameState;
class APartyPlayerState;
class UReturnToMainMenuScreen;
class APartyHUD;
class APartyGameMode;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

UCLASS()
class DEATHMATCHPARTY_API APartyPlayerController : public APlayerController
{
	GENERATED_BODY()

	float MatchTime = 0.0f;
	float WarmUpTime = 0.0f;
	float CooldownTime = 0.0f;
	float LevelStartingTime = 0.0f;
	
	uint32 CountdownInt = 0;

	bool bInitializeCharacterOverlay = false;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;
	int32 HUDGrenades;
	float HUDShield;
	float HUDMaxShield;

	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = true;

	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = true;
	
	UPROPERTY()
	APartyHUD* PartyHUD;
	
	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UPROPERTY()
	APartyGameMode* PartyGameMode;

	UPROPERTY(EditAnywhere, Category=HUD)
	TSubclassOf<UUserWidget> ReturnToMainMenuScreenWidget;

	UPROPERTY()
	UReturnToMainMenuScreen* ReturnToMainMenuScreen;

	bool bReturnToMainMenuOpen = false;
	
	// Ping section
	float HighPingRunningTime = 0.0f;

	UPROPERTY(EditAnywhere)
	float PingLimit = 150.0f;
	
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 5.0f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);
	
	// End ping Section
	
	UFUNCTION()
	void OnRep_MatchState();

	void CheckPing();

public:

	float SingleTripTime = 0.0f;
	
	void SetHUDHealth(float Health, float MaxHealth) const;
	void SetHUDShield(float Shield, float MaxShield) const;
	void SetHUDScore(int32 ScoreAmount) const;
	void SetHUDDefeats(int32 DefeatsAmount) const;
	void SetHUDWeaponAmmo(int32 WeaponAmmoAmount);
	void SetHUDCarriedAmmo(int32 CarriedAmmoAmount);
	void SetHUDTimerCountdown(float CountdownTimer) const;
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	void SetHUDAnnouncementCountdown(float CountdownTime) const;
	void HandleCooldown();
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void ReceivedPlayer() override;
	virtual float GetServerTime();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void DisplayConnectionWarning(bool bDisplayConnectionWarning) const;

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown);

	void HideTeamUIElements() const;
	void InitTeamScores() const;
	void SetHUDRedTeamScore(int32 RedScore) const;
	void SetHUDBlueTeamScore(int32 BlueScore) const;
	

	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);
	
protected:

	float ClientServerDelta = 0.0f;
	float TimeSyncRunningTime = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores() const;

	UPROPERTY(EditAnywhere, Category=Time)
	float TimeSyncFrequency = 5.0f;
	
	virtual void BeginPlay() override;
	void CheckTimeSync(float DeltaSeconds);
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupInputComponent() override;

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	void SetHUDTime();

	UFUNCTION()
	void DisplayReturnToMainMenuScreen();
};
