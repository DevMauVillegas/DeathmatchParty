// Fill out your copyright notice in the Description page of Project Settings.

#include "FlagZone.h"

#include "Components/BoxComponent.h"
#include "GameModes/CaptureFlagGameMode.h"
#include "Weapons/Flag.h"

AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);

	BoxComponent->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		CaptureFlagGameMode = GetWorld()->GetAuthGameMode<ACaptureFlagGameMode>();
	}

	BoxComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &AFlagZone::OnBoxOverlap);
}

void AFlagZone::CapturingTheFlag(AFlag* OverlappingFlag)
{
	CaptureFlagGameMode = CaptureFlagGameMode == nullptr ? GetWorld()->GetAuthGameMode<ACaptureFlagGameMode>() : CaptureFlagGameMode;
	if (CaptureFlagGameMode)
	{
		CaptureFlagGameMode->FlagCaptured(OverlappingFlag, this);
	}
	
	OverlappingFlag->ResetFlag();
}

void AFlagZone::OnBoxOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
                             UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AFlag* OverlappingFlag = Cast<AFlag>(OtherActor))
	{
		if (OverlappingFlag->GetWeaponTeam() != ZoneTeam)
		{
			CapturingTheFlag(OverlappingFlag);
		}
	}
}

