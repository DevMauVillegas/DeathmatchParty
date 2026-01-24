#include "PartyGameMode.h"

#include "Characters/PartyCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "GameState/PartyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/PartyPlayerController.h"
#include "PlayerState/PartyPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

APartyGameMode::APartyGameMode()
{
	bDelayedStart = true;
	
	CountdownTime = 0.0f;

	WarmUpTime = 5.0f;
	
	MatchTime = 120.0f;

	CooldownTime = 10.0f;
	
	LevelStartingTime = 0.0f;

	bIsTeamMatch = false;
}

void APartyGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void APartyGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APartyPlayerController* PC = Cast<APartyPlayerController>(*It))
		{
			PC->OnMatchStateSet(MatchState, bIsTeamMatch);
		}
	}
}

void APartyGameMode::PlayerLeftGame(APartyPlayerState* LeavingPlayerState)
{
	if (LeavingPlayerState == nullptr)
	{
		return;
	}
	
	if (APartyGameState* PartyGameState = GetGameState<APartyGameState>())
	{
		PartyGameState->TopScoringPlayers.Remove(LeavingPlayerState);
	}

	if (APartyCharacter* PlayerLeaving = Cast<APartyCharacter>(LeavingPlayerState->GetPawn()))
	{
		PlayerLeaving->Eliminated(true);
	}
}

float APartyGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

void APartyGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmUpTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.0f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmUpTime  + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.0f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.0f)
		{
			RestartGame();
		}
	}
}

void APartyGameMode::PlayerEliminated(APartyCharacter* EliminatedCharacter, APartyPlayerController* VictimController,
                                      APartyPlayerController* AttackerController)
{
	APartyPlayerState* AttackerPlayerState = AttackerController ? Cast<APartyPlayerState>(AttackerController->PlayerState) : nullptr;
	APartyPlayerState* VictimPlayerState = VictimController ? Cast<APartyPlayerState>(VictimController->PlayerState) : nullptr;
	
	if (AttackerPlayerState && (AttackerPlayerState != VictimPlayerState))
	{
		if (APartyGameState* PartyGameState = GetGameState<APartyGameState>())
		{
			TArray<APartyPlayerState*> PlayerInTheLead;

			// Caching current players in the lead to check if someone lost the lead
			for (auto LeadPlayer : PartyGameState->TopScoringPlayers)
			{
				PlayerInTheLead.Add(LeadPlayer);
			}

			// Adding score to the lead
			AttackerPlayerState->AddToScore(1.f);
			PartyGameState->UpdateTopScores(AttackerPlayerState);

			// Checking if there's a new lead
			if (PartyGameState->TopScoringPlayers.Contains(AttackerPlayerState))
			{
				if (APartyCharacter* Winner = Cast<APartyCharacter>(AttackerPlayerState->GetPawn()))
				{
					Winner->MulticastGainTheLead();
				}
			}

			for (int32 i = 0; i < PlayerInTheLead.Num(); ++i)
			{
				if (PartyGameState->TopScoringPlayers.Contains(PlayerInTheLead[i]))
				{
					continue;
				}
				
				if (APartyCharacter* Loser = Cast<APartyCharacter>(PlayerInTheLead[i]->GetPawn()))
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	EliminatedCharacter->Eliminated(false);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APartyPlayerController* PartyPlayerController = Cast<APartyPlayerController>(*It);
		
		if (PartyPlayerController && AttackerPlayerState && VictimPlayerState)
		{
			PartyPlayerController->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void APartyGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}

	if (EliminatedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num()-1);
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[Selection]);
	}
}
