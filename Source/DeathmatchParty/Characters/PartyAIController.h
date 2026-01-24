// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PartyAIController.generated.h"

class UAISenseConfig_Hearing;
class UAISenseConfig_Sight;
class UAISenseConfig_Damage;
/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API APartyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	
	APartyAIController();

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* AIPerceptionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Damage* DamageSenseConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Sight* SightSenseConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Hearing* HearingSenseConfig;
};
