#include "PlayerController/PartyPlayerController.h"

#include "Characters/PartyCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "GameMechanics/CombatComponent.h"
#include "GameModes/PartyGameMode.h"
#include "GameState/PartyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PartyTypes/Announcements.h"
#include "PlayerState/PartyPlayerState.h"
#include "UI/CharacterOverlay.h"
#include "UI/PartyHUD.h"
#include "UI/ReturnToMainMenuScreen.h"
#include "UI/WarmupAnnouncement.h"


void APartyPlayerController::OnRep_ShowTeamScores() const
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamUIElements();
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

void APartyPlayerController::StartConnectionWarning() const
{
	if (PartyHUD == nullptr)
	{
		return;
	}

	if (UCharacterOverlay* CharacterOverlay = PartyHUD->CharacterOverlay)
	{
		CharacterOverlay->PlayConnectionWarningAnimation();
	}
}

void APartyPlayerController::StopConnectionWarning() const
{
	if (PartyHUD == nullptr)
	{
		return;
	}
	
	if (UCharacterOverlay* CharacterOverlay = PartyHUD->CharacterOverlay)
	{
		CharacterOverlay->StopConnectionWarningAnimation();
	}
}

void APartyPlayerController::HideTeamUIElements() const
{
	if (PartyHUD == nullptr)
	{
		return;
	}

	const UCharacterOverlay* CharacterOverlay = PartyHUD->CharacterOverlay;
	if (CharacterOverlay && CharacterOverlay->TeamHorizontalBox)
	{
		CharacterOverlay->TeamHorizontalBox->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void APartyPlayerController::InitTeamScores() const
{
	if (PartyHUD == nullptr)
	{
		return;
	}

	const UCharacterOverlay* CharacterOverlay = PartyHUD->CharacterOverlay;
	if (CharacterOverlay && CharacterOverlay->RedTeamScore && CharacterOverlay->BlueTeamScore)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), 0);
		CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
		CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void APartyPlayerController::SetHUDRedTeamScore(int32 RedScore) const
{
	if (PartyHUD == nullptr)
	{
		return;
	}

	const UCharacterOverlay* CharacterOverlay = PartyHUD->CharacterOverlay;
	if (CharacterOverlay && CharacterOverlay->RedTeamScore)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void APartyPlayerController::SetHUDBlueTeamScore(int32 BlueScore) const
{
	if (PartyHUD == nullptr)
	{
		return;
	}

	const UCharacterOverlay* CharacterOverlay = PartyHUD->CharacterOverlay;
	if (CharacterOverlay && CharacterOverlay->BlueTeamScore)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
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
	//TODO: move this and the other awful functions to the hud or interface
	
	if (PartyHUD &&
		PartyHUD->CharacterOverlay &&
		PartyHUD->CharacterOverlay->HealthBar &&
		PartyHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		const FString HealthText = FString::Printf(TEXT("%02d"), FMath::CeilToInt(Health));
		PartyHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		PartyHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void APartyPlayerController::SetHUDShield(float Shield, float MaxShield) const
{
	if (PartyHUD &&
		PartyHUD->CharacterOverlay &&
		PartyHUD->CharacterOverlay->ShieldBar &&
		PartyHUD->CharacterOverlay->ShieldText)
	{
		const float ShieldPercent = Shield / MaxShield;
		const FString ShieldText = FString::Printf(TEXT("%02d"), FMath::CeilToInt(Shield));
		PartyHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		PartyHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
}

void APartyPlayerController::SetHUDScore(int32 Score) const
{
	if (PartyHUD &&
		PartyHUD->CharacterOverlay &&
		PartyHUD->CharacterOverlay->Score)
	{
		const FString ScoreText = FString::Printf(TEXT("%02d"), Score);
		PartyHUD->CharacterOverlay->Score->SetText(FText::FromString(ScoreText));
	}
}

void APartyPlayerController::SetHUDDefeats(int32 DefeatsAmount) const
{
	if (PartyHUD &&
		PartyHUD->CharacterOverlay &&
		PartyHUD->CharacterOverlay->Defeat)
	{
		const FString DefeatsText = FString::Printf(TEXT("%02d"), DefeatsAmount);
		PartyHUD->CharacterOverlay->Defeat->SetText(FText::FromString(DefeatsText));
	}
}

void APartyPlayerController::SetHUDWeaponAmmo(int32 WeaponAmmoAmount)
{
	if (PartyHUD &&
		PartyHUD->CharacterOverlay &&
		PartyHUD->CharacterOverlay->WeaponAmmo)
	{
		const FString AmmoText = FString::Printf(TEXT("%02d"), WeaponAmmoAmount);
		PartyHUD->CharacterOverlay->WeaponAmmo->SetText(FText::FromString(AmmoText));

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
	if (PartyHUD &&
		PartyHUD->CharacterOverlay &&
		PartyHUD->CharacterOverlay->TotalAmmo)
	{
		const FString AmmoText = FString::Printf(TEXT("%d"), CarriedAmmoAmount);
		PartyHUD->CharacterOverlay->TotalAmmo->SetText(FText::FromString(AmmoText));

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
	if (PartyHUD &&
		PartyHUD->CharacterOverlay &&
		PartyHUD->CharacterOverlay->TimerCountdown)
	{
		if (CountdownTimer < 0.0f)
		{
			PartyHUD->CharacterOverlay->TimerCountdown->SetText(FText());
			return;
		}
		
		const int32 Minutes = FMath::FloorToInt(CountdownTimer / 60);
		const int32 Seconds = CountdownTimer - Minutes * 60;

		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PartyHUD->CharacterOverlay->TimerCountdown->SetText(FText::FromString(CountdownText));
	}
}

void APartyPlayerController::SetHUDAnnouncementCountdown(float CountdownTime) const
{
	if (PartyHUD &&
		PartyHUD->WarmupWidget &&
		PartyHUD->WarmupWidget->WarmupCountdown)
	{
		if (CountdownTime < 0.0f)
		{
			PartyHUD->WarmupWidget->WarmupCountdown->SetText(FText());
			return;
		}
		
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		const int32 Seconds = CountdownTime - Minutes * 60;

		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PartyHUD->WarmupWidget->WarmupCountdown->SetText(FText::FromString(CountdownText));
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

		if (PartyHUD->WarmupWidget)
		{
			PartyHUD->WarmupWidget->SetVisibility(ESlateVisibility::Hidden);
		}

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

		HighPing ? StartConnectionWarning() : StopConnectionWarning();
		
		ServerReportPingStatus(HighPing);
	}
}

void APartyPlayerController::HandleCooldown()
{
	if (PartyHUD)
	{
		PartyHUD->CharacterOverlay->RemoveFromParent();
		if (PartyHUD->WarmupWidget && PartyHUD->WarmupWidget->WarmupText)
		{
			PartyHUD->WarmupWidget->SetVisibility(ESlateVisibility::Visible);
			FString WarmupText = Announcements::NewMatchStartsIn;
			PartyHUD->WarmupWidget->WarmupText->SetText(FText::FromString(WarmupText));

			APartyGameState* PGS = Cast<APartyGameState>(UGameplayStatics::GetGameState(this));
			if (PGS)
			{
				TArray<APartyPlayerState*> TopPlayers = PGS->TopScoringPlayers;

				const FString MVPText = bShowTeamScores ? GetTeamInfoText(PGS) : GetInfoText(TopPlayers);

				PartyHUD->WarmupWidget->ScoreAnnouncement->SetText(FText::FromString(MVPText));
			}
		}
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

FString APartyPlayerController::GetInfoText(const TArray<APartyPlayerState*>& PlayerStates)
{
	FString MVPText;
	if (PlayerStates.Num() == 0)
	{
		MVPText = Announcements::ThereIsNoWinner;
	}
	else if (PlayerStates.Num() == 1)
	{
		MVPText = FString::Printf(TEXT("MVP: \n%s"), *PlayerStates[0]->GetPlayerName());
	}
	else
	{
		MVPText = Announcements::MultipleWinners;
		for (auto TiedPlayer : PlayerStates)
		{
			MVPText.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}

	return MVPText;
}

FString APartyPlayerController::GetTeamInfoText(APartyGameState* PartyGameState)
{
	if (PartyGameState == nullptr)
	{
		return FString();
	}

	FString InfoString; 

	const int32 RedTeamScore = PartyGameState->RedTeamScore;
	const int32 BlueTeamScore = PartyGameState->BlueTeamScore;

	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoString = Announcements::ThereIsNoWinner;
	}

	else if (RedTeamScore == BlueTeamScore)
	{
		InfoString = Announcements::TeamsTie;
		InfoString.Append(Announcements::RedTeam);
		InfoString.Append(TEXT("\n"));
		InfoString.Append(Announcements::BlueTeam);
	}
	
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoString = Announcements::WinnerTeam;
		InfoString.Append(Announcements::RedTeam);
	}

	else
	{
		InfoString = Announcements::WinnerTeam;
		InfoString.Append(Announcements::BlueTeam);
	}

	return InfoString;
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
