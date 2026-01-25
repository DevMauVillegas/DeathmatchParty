#include "UI/PartyHUD.h"

#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameState/PartyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "PartyTypes/Announcements.h"
#include "PlayerController/PartyPlayerController.h"
#include "PlayerState/PartyPlayerState.h"
#include "UI/CharacterOverlay.h"
#include "UI/ElimAnnouncement.h"
#include "UI/WarmupAnnouncement.h"

void APartyHUD::BeginPlay()
{
	Super::BeginPlay();

	PartyPlayerController = Cast<APartyPlayerController>(GetOwningPlayerController());
	if (PartyPlayerController == nullptr)
	{
		return;
	}
	
	CharacterOverlay = CreateWidget<UCharacterOverlay>(PartyPlayerController, CharacterOverlayClass);
}

void APartyHUD::DrawHUD()
{
	Super::DrawHUD();

	if (GEngine == nullptr)
	{
		return;
	}
	
	FVector2D ViewPortSize;
	GEngine->GameViewport->GetViewportSize(ViewPortSize);

	const FVector2D ViewPortCenter(ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f);

	float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
	
	/*if (HUDPackage.CrosshairCenter)
	{
		FVector2D Spread(0.f, 0.f);
		DrawCrosshair(HUDPackage.CrosshairCenter, ViewPortCenter, Spread, HUDPackage.CrosshairColor);
	}*/
	if (HUDPackage.CrosshairTop)
	{
		FVector2D Spread(0.f, 0.f);
		Spread.Y = -SpreadScaled;
		DrawCrosshair(HUDPackage.CrosshairTop, ViewPortCenter, Spread, HUDPackage.CrosshairColor);
	}
	if (HUDPackage.CrosshairBottom)
	{
		FVector2D Spread(0.f, 0.f);
		Spread.Y = SpreadScaled;
		DrawCrosshair(HUDPackage.CrosshairBottom, ViewPortCenter, Spread, HUDPackage.CrosshairColor);
	}
	if (HUDPackage.CrosshairLeft)
	{
		FVector2D Spread(0.f, 0.f);
		Spread.X = -SpreadScaled;
		DrawCrosshair(HUDPackage.CrosshairLeft, ViewPortCenter, Spread, HUDPackage.CrosshairColor);
	}
	if (HUDPackage.CrosshairRight)
	{
		FVector2D Spread(0.f, 0.f);
		Spread.X = SpreadScaled;
		DrawCrosshair(HUDPackage.CrosshairRight, ViewPortCenter, Spread, HUDPackage.CrosshairColor);
	}
}

void APartyHUD::AddCharacterOverlay()
{
	if (CharacterOverlay)
	{
		CharacterOverlay->AddToViewport();

		WarmupWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void APartyHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && WarmupWidgetClass)
	{
		WarmupWidget = CreateWidget<UWarmupAnnouncement>(PlayerController, WarmupWidgetClass);
		if (WarmupWidget)
		{
			WarmupWidget->AddToViewport();
		}
	}
}

void APartyHUD::AddElimAnnouncement(const FString& AttackerName, const FString& VictimName)
{
	if (PartyPlayerController && ElimAnnouncementClass)
	{
		if (UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(PartyPlayerController, ElimAnnouncementClass))
		{
			ElimAnnouncementWidget->SetElimAnnouncementText(AttackerName, VictimName);
			ElimAnnouncementWidget->AddToViewport();

			for (const auto Element : ElimElements)
			{
				if (Element && Element->ElimHorizontalBox)
				{
					if (UCanvasPanelSlot* CanvasPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Cast<UWidget>(Element->ElimHorizontalBox)))
					{
						const FVector2D Position = CanvasPanelSlot->GetPosition();
						const FVector2D NewPosition(Position.X, Position.Y - CanvasPanelSlot->GetSize().Y);

						CanvasPanelSlot->SetPosition(NewPosition);
					}
				}
			}

			ElimElements.Add(ElimAnnouncementWidget);
			
			FTimerHandle ElimAnnouncementTimer;
			FTimerDelegate ElimElementDelegate;
			ElimElementDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"),ElimAnnouncementWidget);

			GetWorldTimerManager().SetTimer(
				ElimAnnouncementTimer,
				ElimElementDelegate,
				ElimAnnouncementTime,
				false);
		}
		
	}
}

void APartyHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor);
}

void APartyHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* ElementToRemove)
{
	if (ElementToRemove)
	{
		ElementToRemove->RemoveFromParent();
	}
}

void APartyHUD::SetShield(const float Current, const float Max)
{
	const float ShieldPercent = Current / Max;
	CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
	
	const int32 CeilShield = FMath::CeilToInt(Current);
	CharacterOverlay->ShieldText->SetText(FText::AsNumber(CeilShield));
}

void APartyHUD::SetHealth(const float Current, const float Max)
{
	const float HealthPercent = Current / Max;
	CharacterOverlay->HealthBar->SetPercent(HealthPercent);
	
	const int32 CeilHealth = FMath::CeilToInt(Current);
	CharacterOverlay->HealthText->SetText(FText::AsNumber(CeilHealth));
}

void APartyHUD::SetMatchTime(float NewTimerValue)
{
	if (NewTimerValue < 0.0f)
	{
		CharacterOverlay->TimerCountdown->SetText(FText());
		return;
	}
		
	const int32 Minutes = FMath::FloorToInt(NewTimerValue / 60);
	const int32 Seconds = NewTimerValue - Minutes * 60;

	const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	CharacterOverlay->TimerCountdown->SetText(FText::FromString(CountdownText));
}

void APartyHUD::HandleCooldown()
{
	CharacterOverlay->RemoveFromParent();
	if (WarmupWidget && WarmupWidget->WarmupText)
	{
		WarmupWidget->SetVisibility(ESlateVisibility::Visible);
		FString WarmupText = Announcements::NewMatchStartsIn;
		WarmupWidget->WarmupText->SetText(FText::FromString(WarmupText));

		APartyGameState* PGS = Cast<APartyGameState>(UGameplayStatics::GetGameState(this));
		if (PGS)
		{
			TArray<APartyPlayerState*> TopPlayers = PGS->TopScoringPlayers;

			const FString MVPText = bShowTeamScores ? GetTeamInfoText(PGS) : GetInfoText(TopPlayers);

			WarmupWidget->ScoreAnnouncement->SetText(FText::FromString(MVPText));
		}
	}
}

void APartyHUD::DisplayConnectionWarning(const bool bDisplayConnectionWarning)
{
	bDisplayConnectionWarning ?
		CharacterOverlay->PlayConnectionWarningAnimation() :
		CharacterOverlay->StopConnectionWarningAnimation();
}

void APartyHUD::ShowTeamScoresUpdated(const bool NewShowTeamScores)
{
	bShowTeamScores = NewShowTeamScores;

	if (bShowTeamScores)
	{
		SetRedTeamScore(0);
		SetBlueTeamScore(0);
	}
	else
	{
		HideTeamUIElements();
	}
}

void APartyHUD::SetScore(const int32 NewScore)
{
	CharacterOverlay->Score->SetText(FText::AsNumber(NewScore));
}

void APartyHUD::SetDefeats(const int32 NewDefeats)
{
	CharacterOverlay->Defeat->SetText(FText::AsNumber(NewDefeats));
}

void APartyHUD::SetWeaponAmmo(const int32 NewWeaponAmmo)
{
	CharacterOverlay->WeaponAmmo->SetText(FText::AsNumber(NewWeaponAmmo));
}

void APartyHUD::SetCarriedAmmo(const int32 NewCarriedAmmo)
{
	CharacterOverlay->TotalAmmo->SetText(FText::AsNumber(NewCarriedAmmo));
}

void APartyHUD::SetRedTeamScore(const int32 NewRedTeamScore)
{
	CharacterOverlay->RedTeamScore->SetText(FText::AsNumber(NewRedTeamScore));
}

void APartyHUD::SetBlueTeamScore(const int32 NewBlueTeamScore)
{
	CharacterOverlay->BlueTeamScore->SetText(FText::AsNumber(NewBlueTeamScore));
}

void APartyHUD::HideTeamUIElements()
{
	if (CharacterOverlay && CharacterOverlay->TeamHorizontalBox)
	{
		CharacterOverlay->TeamHorizontalBox->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void APartyHUD::SetAnnouncementCountdown(const float NewAnnouncementCountdownTime)
{
	if (NewAnnouncementCountdownTime < 0.0f)
	{
		WarmupWidget->WarmupCountdown->SetText(FText());
		return;
	}
		
	const int32 Minutes = FMath::FloorToInt(NewAnnouncementCountdownTime / 60);
	const int32 Seconds = NewAnnouncementCountdownTime - Minutes * 60;

	const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	WarmupWidget->WarmupCountdown->SetText(FText::FromString(CountdownText));
}

FString APartyHUD::GetTeamInfoText(const APartyGameState* PartyGameState)
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

FString APartyHUD::GetInfoText(const TArray<APartyPlayerState*>& PlayerStates)
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