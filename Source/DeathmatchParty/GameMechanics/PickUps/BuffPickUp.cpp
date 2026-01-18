
#include "GameMechanics/PickUps/BuffPickUp.h"

#include "NiagaraFunctionLibrary.h"
#include "Characters/PartyCharacter.h"
#include "GameMechanics/BuffComponent.h"

ABuffPickUp::ABuffPickUp()
{
	bReplicates = true;

	OverlappingActorLocation = FVector::Zero();

	EffectAmount = 2.0f;
	EffectDuration = 5.0f;
	EffectPeriod = 1.0f;
}

void ABuffPickUp::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlapComponent, OtherActor, OtherComponent, OtherBodyIndex, bFromSweep, SweepResult);

	if (const APartyCharacter* PartyCharacter = Cast<APartyCharacter>(OtherActor))
	{
		UAbilitySystemComponent* AbilitySystemComponent = PartyCharacter->GetAbilitySystemComponent();

		if (AbilitySystemComponent == nullptr)
		{
			return;
		}
		
		if (IsValid(GameplayEffect))
		{
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect.Get(), 1.0f, EffectContext);

			if (SpecHandle.IsValid())
			{
				FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
				Spec->SetSetByCallerMagnitude(EffectTag, EffectAmount);
				Spec->SetDuration(EffectDuration, true);
				Spec->Period = EffectPeriod;
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec);
			}
		}
	}
	
	Destroy();
}

void ABuffPickUp::Destroyed()
{
	if (PickUpEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickUpEffect, GetActorLocation(), GetActorRotation());
	}
	
	Super::Destroyed();
}
