// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

class APartyCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DEATHMATCHPARTY_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY()
	APartyCharacter* Character;

	// SPEED BUFF
	FTimerHandle SpeedBuffTimer;
	float InitialBaseSpeed;
	float InitialCrouchSpeed;
	void ResetSpeeds();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchedSpeed);

	// JUMP BUFF

	FTimerHandle JumpBuffTimerHandler;
	float InitialJumpVelocity;

	void ResetJump();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);
	
public:
	
	UBuffComponent();
	friend APartyCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	void SetInitialJumpVelocity(float Velocity);

	void SpeedUp(float BuffedBaseSpeed, float BuffedCrouchSpeed, float Time);
	
	void BuffJump(float BuffJumpVelocity, float JumpBuffTime);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
