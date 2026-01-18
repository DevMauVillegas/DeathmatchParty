// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PartyCharacter.h"
#include "AIPartyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class DEATHMATCHPARTY_API AAIPartyCharacter : public APartyCharacter
{
	GENERATED_BODY()

	virtual void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser) override;

	
};
