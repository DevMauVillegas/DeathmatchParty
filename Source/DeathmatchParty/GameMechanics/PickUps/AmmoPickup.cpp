#include "GameMechanics/PickUps/AmmoPickup.h"
#include "Characters/PartyCharacter.h"
#include "GameMechanics/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlapComponent, OtherActor, OtherComponent, OtherBodyIndex, bFromSweep, SweepResult);

	if (const APartyCharacter* PartyCharacter = Cast<APartyCharacter>(OtherActor))
	{
		if (UCombatComponent* CombatComponent = PartyCharacter->GetCombatComponent())
		{
			CombatComponent->PickUpAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}
