// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMechanics/PickUps/JumpPickUp.h"

#include "Characters/PartyCharacter.h"
#include "GameMechanics/BuffComponent.h"

AJumpPickUp::AJumpPickUp() :
	JumpVelocityBuff(4000.0f),
	JumpVelocityTime(20.0f)
{
	
}

void AJumpPickUp::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlapComponent, OtherActor, OtherComponent, OtherBodyIndex, bFromSweep, SweepResult);

	if (const APartyCharacter* PartyCharacter = Cast<APartyCharacter>(OtherActor))
	{
		OverlappingActorLocation = PartyCharacter->GetActorLocation();

		if (UBuffComponent* BuffComponent = PartyCharacter->GetBuffComponent())
		{
			BuffComponent->BuffJump(JumpVelocityBuff, JumpVelocityTime);
		}
	}
	
	Destroy();
}
