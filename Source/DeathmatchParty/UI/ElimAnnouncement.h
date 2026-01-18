// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ElimAnnouncement.generated.h"

class UTextBlock;
class UHorizontalBox;

/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API UElimAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetElimAnnouncementText(const FString& AttackerName, const FString& VictimName) const;

	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* ElimHorizontalBox;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ElimPlayerText;
};
