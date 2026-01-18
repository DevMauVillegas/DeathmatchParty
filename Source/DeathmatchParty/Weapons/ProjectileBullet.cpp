#include "Weapons/ProjectileBullet.h"

#include "Characters/PartyCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameMechanics/LagCompensationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/PartyPlayerController.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->InitialSpeed = ProjectileInitialSpeed;
	ProjectileMovementComponent->SetIsReplicated(true);
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, ProjectileInitialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = ProjectileInitialSpeed;
		}
	}
}
#endif

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (const APartyCharacter* OwnerCharacter = Cast<APartyCharacter>(GetOwner()))
	{
		if (APartyPlayerController* OwnerController = Cast<APartyPlayerController>(OwnerCharacter->Controller))
		{
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				bool bIsHeadshot = Hit.BoneName.ToString() == FString("head");
				const float DamageToApply = bIsHeadshot ? HeadShotDamage : Damage;
				
				UGameplayStatics::ApplyDamage(OtherActor, DamageToApply, OwnerController, this, UDamageType::StaticClass());
				Super::OnHit(HitComponent, OtherActor, OtherComponent, NormalImpulse, Hit);
				return;
			}

			APartyCharacter* OtherCharacter = Cast<APartyCharacter>(OtherActor);
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled() && OtherCharacter)
			{
				OwnerCharacter->GetLagCompensationComponent()->ProjectileServerScoreRequest(
					OtherCharacter,
					TraceStart,
					InitialVelocity,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime
				);
			}
		}
	}

	Super::OnHit(HitComponent, OtherActor, OtherComponent, NormalImpulse, Hit);
}
