// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"

#include "Characters/PartyCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AFlag::AFlag()
{
	bReplicates = true;
	
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	SetRootComponent(FlagMesh);

	GetAreaSphere()->SetupAttachment(FlagMesh);
	GetPickupWidget()->SetupAttachment(FlagMesh);
	
	FlagMesh->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FlagMesh->SetIsReplicated(true);	
}

void AFlag::SetFlagVisibility(bool bIsVisible) const
{
	if (FlagMesh)
	{
		FlagMesh->SetVisibility(bIsVisible);
	}
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();
	
	SetReplicateMovement(true);
	InitialTransform = GetActorTransform();
}

void AFlag::OnEquipped()
{
	DisplayPickupWidget(false);
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

void AFlag::OnDropped()
{
	if (HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	PartyCharacterOwner = nullptr;
	PartyControllerOwner = nullptr;
}


void AFlag::ResetFlag()
{
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (APartyCharacter* FlagOwner = Cast<APartyCharacter>(GetOwner()))
	{
		FlagOwner->SetHoldingTheFlag(false);
		FlagOwner->SetOverlappingWeapon(nullptr);
		FlagOwner->UnCrouch();
	}

	// For whatever reason the things are never replicated properly.
	// So fuck it, everyone can teleport it
	
	/*
	if (!HasAuthority())
	{
		return;
	}*/
	
	SetActorTransform(InitialTransform);
	
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagMesh->DetachFromComponent(DetachRules);
	
	SetOwner(nullptr);
	PartyCharacterOwner = nullptr;
	PartyControllerOwner = nullptr;
	
	SetWeaponState(EWeaponState::EWS_Initial);
}
