#include "PlayerController/PartyPlayerController.h"

#include "Characters/PartyCharacter.h"
#include "GameFramework/GameMode.h"
#include "GameMechanics/CombatComponent.h"
#include "GameModes/PartyGameMode.h"
#include "GameState/PartyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/PartyPlayerState.h"
#include "UI/PartyHUD.h"
#include "UI/ReturnToMainMenuScreen.h"


void APartyPlayerController::OnRep_ShowTeamScores() const
{
	if (PartyHUD)
	{
		PartyHUD->ShowTeamScoresUpdated(bShowTeamScores);
	}
}

void APartyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PartyHUD = Cast<APartyHUD>(GetHUD());
	
	ServerCheckMatchState();
}

void APartyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetHUDTime();
	CheckTimeSync(DeltaSeconds);

	HighPingRunningTime += DeltaSeconds;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		CheckPing();
	}
}

void APartyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (const APartyCharacter* PartyCharacter = Cast<APartyCharacter>(InPawn); PartyCharacter->GetAttributeSet())
	{
		SetHUDHealth(PartyCharacter->GetAttributeSet()->GetHealth(), PartyCharacter->GetAttributeSet()->GetMaxHealth());
		SetHUDShield(PartyCharacter->GetAttributeSet()->GetShield(), PartyCharacter->GetAttributeSet()->GetMaxShield());
	}
}

void APartyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void APartyPlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	if (TimeSyncRunningTime > TimeSyncFrequency)
	{
		TimeSyncRunningTime = 0.0f;
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void APartyPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void APartyPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	const float CurrentServerTime = TimeServerReceivedClientRequest + (RoundTripTime * 0.5f);

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float APartyPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void APartyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APartyPlayerController, MatchState);
	DOREPLIFETIME(APartyPlayerController, bShowTeamScores);
}


//
// HUD MODIFICATIONS
//

void APartyPlayerController::DisplayConnectionWarning(const bool bDisplayConnectionWarning) const
{
	if (PartyHUD == nullptr)
	{
		return;
	}
	
	PartyHUD->DisplayConnectionWarning(bDisplayConnectionWarning);
}

void APartyPlayerController::HideTeamUIElements() const
{
	if (PartyHUD == nullptr)
	{
		return;
	}

	PartyHUD->HideTeamUIElements();
}

void APartyPlayerController::InitTeamScores() const
{
	SetHUDRedTeamScore(0);
	SetHUDBlueTeamScore(0);
}

void APartyPlayerController::SetHUDRedTeamScore(int32 RedScore) const
{
	if (PartyHUD == nullptr)
	{
		return;
	}

	PartyHUD->SetRedTeamScore(RedScore);
}

void APartyPlayerController::SetHUDBlueTeamScore(int32 BlueScore) const
{
	if (PartyHUD == nullptr)
	{
		return;
	}

	PartyHUD->SetBlueTeamScore(BlueScore);
}

void APartyPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void APartyPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();

	if (!Attacker || !Victim || !Self)
	{
		return;
	}
	
	if (PartyHUD)
	{
		FString AttackerString;
		FString VictimString;
		
		if (Attacker == Self)
		{
			AttackerString = "You";

			if (Attacker == Victim)
			{
				VictimString = "yourself";
			}
			else
			{
				VictimString = Victim->GetPlayerName();
			}
		}
		
		else
		{
			AttackerString = Attacker->GetPlayerName();
			
			if (Self == Victim)
			{
				VictimString = "you";
			}

			else if (Attacker == Victim)
			{
				VictimString = "itself";
			}
			else
			{
				VictimString = Victim->GetPlayerName();
			}
		}

		PartyHUD->AddElimAnnouncement(AttackerString, VictimString);
	}
}

void APartyPlayerController::SetHUDTime()
{
	float TimeLeft = 0.0f;
	
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmUpTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmUpTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + WarmUpTime + MatchTime - GetServerTime() + LevelStartingTime;
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	
	if (HasAuthority())
	{
		PartyGameMode = PartyGameMode == nullptr ? Cast<APartyGameMode>(UGameplayStatics::GetGameMode(this)) : PartyGameMode;

		if (PartyGameMode)
		{
			SecondsLeft = FMath::CeilToInt(PartyGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDTimerCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void APartyPlayerController::DisplayReturnToMainMenuScreen()
{
	if (ReturnToMainMenuScreenWidget == nullptr)
	{
		return;
	}

	if (ReturnToMainMenuScreen == nullptr)
	{
		ReturnToMainMenuScreen = CreateWidget<UReturnToMainMenuScreen>(this, ReturnToMainMenuScreenWidget);
	}

	if (ReturnToMainMenuScreen != nullptr)
	{
		bReturnToMainMenuOpen = ! bReturnToMainMenuOpen;
		bReturnToMainMenuOpen ? ReturnToMainMenuScreen->MenuSetup() : ReturnToMainMenuScreen->MenuTearDown();
	}
}

void APartyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent == nullptr)
	{
		return;
	}

	InputComponent->BindAction("Quit", IE_Pressed, this, &APartyPlayerController::DisplayReturnToMainMenuScreen);
}

void APartyPlayerController::SetHUDHealth(float Health, float MaxHealth) const
{
	if (PartyHUD)
	{
		PartyHUD->SetHealth(Health, MaxHealth);
	}
}

void APartyPlayerController::SetHUDShield(float Shield, float MaxShield) const
{
	if (PartyHUD)
	{
		PartyHUD->SetShield(Shield, MaxShield);
	}
}

void APartyPlayerController::SetHUDScore(int32 Score) const
{
	if (PartyHUD)
	{
		PartyHUD->SetScore(Score);
	}
}

void APartyPlayerController::SetHUDDefeats(int32 DefeatsAmount) const
{
	if (PartyHUD)
	{
		PartyHUD->SetDefeats(DefeatsAmount);
	}
}

void APartyPlayerController::SetHUDWeaponAmmo(int32 WeaponAmmoAmount)
{
	if (PartyHUD)
	{
		PartyHUD->SetWeaponAmmo(WeaponAmmoAmount);

		bInitializeWeaponAmmo = false;
	}
	else
	{
		HUDWeaponAmmo = WeaponAmmoAmount;
		bInitializeWeaponAmmo = true;
	}
}

void APartyPlayerController::SetHUDCarriedAmmo(int32 CarriedAmmoAmount)
{
	if (PartyHUD)
	{
		PartyHUD->SetCarriedAmmo(CarriedAmmoAmount);

		bInitializeCarriedAmmo = false;
	}
	else
	{
		HUDCarriedAmmo = CarriedAmmoAmount;
		bInitializeCarriedAmmo = true;
	}
}

void APartyPlayerController::SetHUDTimerCountdown(float CountdownTimer) const
{
	if (PartyHUD)
	{
		PartyHUD->SetMatchTime(CountdownTimer);
	}
}

void APartyPlayerController::SetHUDAnnouncementCountdown(float CountdownTime) const
{
	if (PartyHUD)
	{
		PartyHUD->SetAnnouncementCountdown(CountdownTime);
	}
}

void APartyPlayerController::HandleMatchHasStarted(bool bTeamsMatch/* = false*/)
{
	if (HasAuthority())
	{
		bShowTeamScores = bTeamsMatch;
	}
	
	if (PartyHUD)
	{
		PartyHUD->AddCharacterOverlay();

		if (!HasAuthority())
		{
			return;
		}
		
		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamUIElements();
		}
	}
}

//
// CHANGES IN GAME STATE 
//

void APartyPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void APartyPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void APartyPlayerController::CheckPing()
{
	HighPingRunningTime = 0;

	if (PlayerState)
	{
		float MilisecondsPing = PlayerState->GetPingInMilliseconds();

		const bool HighPing = PingLimit < MilisecondsPing;

		DisplayConnectionWarning(HighPing);
		
		ServerReportPingStatus(HighPing);
	}
}

void APartyPlayerController::HandleCooldown()
{
	if (PartyHUD)
	{
		PartyHUD->HandleCooldown();
	}

	if (APartyCharacter* PC = Cast<APartyCharacter>(GetPawn()))
	{
		PC->bDisabledGameplay = true;
		if (auto Combat = PC->GetCombatComponent())
		{
			Combat->FireButtonPressed(false);
		}
	}
}

void APartyPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch/* = false*/)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void APartyPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown)
{
	WarmUpTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	CooldownTime = Cooldown;

	OnMatchStateSet(MatchState);

	if (PartyHUD && (MatchState == MatchState::WaitingToStart) && !HasAuthority())
	{
		PartyHUD->AddAnnouncement();
	}
}

void APartyPlayerController::ServerCheckMatchState_Implementation()
{
	if (const APartyGameMode* GM = Cast<APartyGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		WarmUpTime = GM->WarmUpTime;
		MatchTime = GM->MatchTime;
		LevelStartingTime = GM->LevelStartingTime;
		CooldownTime = GM->CooldownTime;
		
		MatchState = GM->GetMatchState();
		ClientJoinMidgame(MatchState, WarmUpTime, MatchTime, LevelStartingTime, CooldownTime);
	}
	
	if (PartyHUD && (MatchState == MatchState::WaitingToStart))
	{
		PartyHUD->AddAnnouncement();
	}
}
