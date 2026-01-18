// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMechanics/StartupGameMode.h"

#include "GameFramework/GameUserSettings.h"

void AStartupGameMode::BeginPlay()
{
	Super::BeginPlay();

	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	UserSettings->RunHardwareBenchmark();
	UserSettings->ApplyHardwareBenchmarkResults();
}
