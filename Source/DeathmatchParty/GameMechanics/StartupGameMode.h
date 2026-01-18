// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StartupGameMode.generated.h"

/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API AStartupGameMode : public AGameModeBase
{
	GENERATED_BODY()

	virtual void BeginPlay() override;
	
};
