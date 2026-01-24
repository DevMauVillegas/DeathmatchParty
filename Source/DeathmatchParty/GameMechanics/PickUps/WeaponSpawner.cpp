// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponSpawner.h"
#include "Weapons/Weapon.h"

AWeaponSpawner::AWeaponSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SpawnPickUpTimerMin = 0.0f;
	
	SpawnPickUpTimerMax = 9999.9f;

	SpawnedPickUp = nullptr;
}

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
