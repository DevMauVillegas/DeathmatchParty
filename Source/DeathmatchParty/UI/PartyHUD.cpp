#include "UI/PartyHUD.h"

#include "AbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Characters/PartyCharacter.h"
#include "Components/CanvasPanelSlot.h"
#include "PlayerController/PartyPlayerController.h"
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

