// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSpawner.h"

#include "Weapons/Weapon.h"

// Sets default values
AWeaponSpawner::AWeaponSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWeaponSpawner::BeginPlay()
{
	Super::BeginPlay();

	SpawnPickUpTimerStart();
}

void AWeaponSpawner::SpawnPickUp()
{
	const int32 NumPickClasses = PickUpClasses.Num();

	if (NumPickClasses > 0)
	{
		const int32 Selection = FMath::RandRange(0, NumPickClasses-1);
		
		SpawnedPickUp = GetWorld()->SpawnActor<AWeapon>(PickUpClasses[Selection], GetActorTransform());
		
		if (HasAuthority() && SpawnedPickUp)
		{
			SpawnedPickUp->OnWeaponPickedUp.AddUniqueDynamic(this, &AWeaponSpawner::SpawnPickUpTimerStart);
		}
	}
}

void AWeaponSpawner::SpawnPickUpTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickUp();
	}
}

void AWeaponSpawner::SpawnPickUpTimerStart()
{
	const float SpawnTime = FMath::FRandRange(SpawnPickUpTimerMin, SpawnPickUpTimerMax);

	GetWorldTimerManager().SetTimer(SpawnPickUpTimer, this, &AWeaponSpawner::SpawnPickUpTimerFinished, SpawnTime);

}

// Called every frame
void AWeaponSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

