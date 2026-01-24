// Fill out your copyright notice in the Description page of Project Settings.


#include "AIPartyCharacter.h"
#include "Perception/AISense_Damage.h"

void AAIPartyCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                      AController* InstigatorController, AActor* DamageCauser)
{
	if (HasAuthority())
	{
		UAISense_Damage::ReportDamageEvent(
			GetWorld(),
			this,
			InstigatorController,
			Damage,
			DamageCauser->GetActorLocation(),
			DamageCauser->GetActorLocation()
		);
	}
	
	Super::ReceiveDamage(DamagedActor, Damage, DamageType, InstigatorController, DamageCauser);
}
