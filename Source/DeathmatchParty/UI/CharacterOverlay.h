// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "CharacterOverlay.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;

UCLASS()
class DEATHMATCHPARTY_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	

	void PlayConnectionWarningAnimation();
	void StopConnectionWarningAnimation();

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Score;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Defeat;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalAmmo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimerCountdown;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlueTeamScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RedTeamScore;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* TeamHorizontalBox;

	UPROPERTY(meta = (BindWidget))
	UImage* ConnectionWarning;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* BlinkConnectionWarning;
};
