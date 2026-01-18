// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WarmupAnnouncement.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API UWarmupAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WarmupCountdown;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WarmupText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAnnouncement;
};
