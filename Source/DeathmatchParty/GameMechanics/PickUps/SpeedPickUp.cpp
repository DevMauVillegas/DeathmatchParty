
#include "GameMechanics/PickUps/SpeedPickUp.h"

#include "Characters/PartyCharacter.h"
#include "GameMechanics/BuffComponent.h"

void ASpeedPickUp::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlapComponent, OtherActor, OtherComponent, OtherBodyIndex, bFromSweep, SweepResult);

	if (const APartyCharacter* PartyCharacter = Cast<APartyCharacter>(OtherActor))
	{
		OverlappingActorLocation = PartyCharacter->GetActorLocation();

		UBuffComponent* BuffComponent = PartyCharacter->GetBuffComponent();
		BuffComponent->SpeedUp(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
	}
	
	Destroy();
}
