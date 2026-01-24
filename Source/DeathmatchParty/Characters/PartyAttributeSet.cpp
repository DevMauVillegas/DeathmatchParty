// Fill out your copyright notice in the Description page of Project Settings.


#include "PartyAttributeSet.h"
#include "Net/UnrealNetwork.h"

UPartyAttributeSet::UPartyAttributeSet()
{
	Health = 100.0f;
	MaxHealth = 100.0f;
	Shield = 0.0f;
	Speed = 550.0f;
	CrouchSpeed = 330.0f;
	JumpSpeed = 100.0f;
	FireRate = 16.0f;
}

void UPartyAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPartyAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPartyAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPartyAttributeSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPartyAttributeSet, Speed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPartyAttributeSet, JumpSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPartyAttributeSet, FireRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPartyAttributeSet, CrouchSpeed, COND_None, REPNOTIFY_Always);
}

void UPartyAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
	}
}


void UPartyAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPartyAttributeSet, Health, OldHealth);
}

void UPartyAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPartyAttributeSet, MaxHealth, OldMaxHealth);
}

void UPartyAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPartyAttributeSet, Shield, OldShield);
}

void UPartyAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPartyAttributeSet, MaxShield, OldMaxShield);
}

void UPartyAttributeSet::OnRep_Speed(const FGameplayAttributeData& OldSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPartyAttributeSet, Speed, OldSpeed);
}

void UPartyAttributeSet::OnRep_JumpSpeed(const FGameplayAttributeData& OldJumpSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPartyAttributeSet, JumpSpeed, OldJumpSpeed);
}

void UPartyAttributeSet::OnRep_FireRate(const FGameplayAttributeData& OldFireRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPartyAttributeSet, FireRate, OldFireRate);
}

void UPartyAttributeSet::OnRep_CrouchSpeed(const FGameplayAttributeData& OldCrouchSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPartyAttributeSet, CrouchSpeed, OldCrouchSpeed);
}


