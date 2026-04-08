// Fill out your copyright notice in the Description page of Project Settings.


#include "PartyAIController.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

APartyAIController::APartyAIController()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	
	DamageSenseConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageSenseConfig"));
	DamageSenseConfig->SetMaxAge(0.5f);
	AIPerceptionComp->ConfigureSense(*DamageSenseConfig);
	
	SightSenseConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightSenseConfig"));
	SightSenseConfig->SightRadius = 1500;
	SightSenseConfig->PeripheralVisionAngleDegrees = 65;
	SightSenseConfig->SetStartsEnabled(true);
	SightSenseConfig->SetMaxAge(5.0f);  
	SightSenseConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightSenseConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightSenseConfig->DetectionByAffiliation.bDetectFriendlies = true;
	
	AIPerceptionComp->ConfigureSense(*SightSenseConfig);
	
	HearingSenseConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingSenseConfig->HearingRange = 3000.0f;           // How far AI can hear
	HearingSenseConfig->SetMaxAge(5.0f);                  // How long AI can remember things heard
	HearingSenseConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingSenseConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingSenseConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerceptionComp->ConfigureSense(*HearingSenseConfig);
	
	AIPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}
