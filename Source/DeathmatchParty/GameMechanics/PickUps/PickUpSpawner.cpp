
#include "GameMechanics/PickUps/PickUpSpawner.h"

#include "GameMechanics/PickUps/PickUp.h"

// Sets default values
APickUpSpawner::APickUpSpawner():
	SpawnPickUpTimerMin(0),
	SpawnPickUpTimerMax(0),
	SpawnedPickUp(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickUpSpawner::BeginPlay()
{
	Super::BeginPlay();

	SpawnPickUpTimerStart((AActor*)nullptr);

}

void APickUpSpawner::SpawnPickUp()
{
	const int32 NumPickClasses = PickUpClasses.Num();

	if (NumPickClasses > 0)
	{
		const int32 Selection = FMath::RandRange(0, NumPickClasses-1);
		
		SpawnedPickUp = GetWorld()->SpawnActor<APickUp>(PickUpClasses[Selection], GetActorTransform());
		
		if (HasAuthority() && SpawnedPickUp)
		{
			SpawnedPickUp->OnDestroyed.AddDynamic(this, &APickUpSpawner::SpawnPickUpTimerStart);
		}
	}
}

void APickUpSpawner::SpawnPickUpTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickUp();
	}
}

void APickUpSpawner::SpawnPickUpTimerStart(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnPickUpTimerMin, SpawnPickUpTimerMax);

	GetWorldTimerManager().SetTimer(SpawnPickUpTimer, this, &APickUpSpawner::SpawnPickUpTimerFinished, SpawnTime);
}

void APickUpSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

