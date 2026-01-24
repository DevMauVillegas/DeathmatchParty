#include "GameMechanics/BuffComponent.h"

#include "Characters/PartyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	Character = nullptr;
	InitialBaseSpeed = 0;
	InitialCrouchSpeed = 0;
	InitialJumpVelocity = 0;
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::BuffJump(float JumpVelocity, float JumpBuffTime)
{
	Character->GetWorldTimerManager().SetTimer(JumpBuffTimerHandler, this, &UBuffComponent::ResetJump, JumpBuffTime);

	MulticastJumpBuff_Implementation(JumpVelocity);
}

void UBuffComponent::ResetJump()
{
	MulticastJumpBuff_Implementation(InitialJumpVelocity);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (Character == nullptr)
	{
		return;
	}

	if (const auto CharacterMovement = Character->GetCharacterMovement())
	{
		CharacterMovement->JumpZVelocity = JumpVelocity;
	}
}

void UBuffComponent::SpeedUp(float BuffedBaseSpeed, float BuffedCrouchSpeed, float Time)
{
	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeeds, Time);
	MulticastSpeedBuff_Implementation(BuffedBaseSpeed, BuffedCrouchSpeed);
}

void UBuffComponent::ResetSpeeds()
{
	MulticastSpeedBuff_Implementation(InitialBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchedSpeed)
{
	const auto CharacterMovement = Character->GetCharacterMovement();
	CharacterMovement->MaxWalkSpeed = BaseSpeed;
	CharacterMovement->MaxWalkSpeedCrouched = CrouchedSpeed;
}
