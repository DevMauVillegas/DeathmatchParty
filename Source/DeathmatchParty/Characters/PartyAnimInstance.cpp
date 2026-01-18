#include "Characters/PartyAnimInstance.h"
#include "PartyTypes/CombatState.h"
#include "Characters/PartyCharacter.h"
#include "Weapons/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

void UPartyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PartyCharacter = Cast<APartyCharacter>(TryGetPawnOwner());
}

void UPartyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (PartyCharacter == nullptr)
	{
		PartyCharacter = Cast<APartyCharacter>(TryGetPawnOwner());
	}

	if (PartyCharacter == nullptr)
	{
		return;
	}

	FVector Velocity = PartyCharacter->GetVelocity();
	Velocity.Z = 0.0f;
	Speed = Velocity.Size();

	const auto CharacterMovement = PartyCharacter->GetCharacterMovement();
	
	if (CharacterMovement == nullptr)
	{
		return;
	}

	bIsInAir = CharacterMovement->IsFalling();

	bIsAccelerating = CharacterMovement->GetCurrentAcceleration().Size() > 0;
	bIsWeaponEquipped = PartyCharacter->IsWeaponEquipped();
	EquippedWeapon = PartyCharacter->GetEquippedWeapon();
	bIsCrouched = PartyCharacter->bIsCrouched;
	bIsAiming = PartyCharacter->IsAiming();
	TurningInPlace = PartyCharacter->GetTurningInPlace();
	bRotateRootBone = PartyCharacter->ShouldRotateRootBone();
	bIsEliminated = PartyCharacter->IsEliminated();
	bHoldingFlag = PartyCharacter->IsHoldingTheFlag();
	
	bUseFABRIK = PartyCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;

	bUseAimOffsets = PartyCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	
	FRotator AimRotation = PartyCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PartyCharacter->GetVelocity());
	YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = PartyCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = PartyCharacter->GetAO_Yaw();
	AO_Pitch = PartyCharacter->GetAO_Pitch();
	
	if (bIsWeaponEquipped == false && EquippedWeapon == nullptr)
	{
		return;
	}
	
	if (auto WeaponMesh = EquippedWeapon->GetWeaponMesh())
	{
		if (auto CharacterMesh = PartyCharacter->GetMesh())
		{
			LHandTransform = WeaponMesh->GetSocketTransform(FName("LeftHandSocket"), RTS_World);
			FVector OutPosition;
			FRotator OutRotation;
			CharacterMesh->TransformToBoneSpace(
				FName("hand_r"),
				LHandTransform.GetLocation(),
				FRotator::ZeroRotator,
				OutPosition,
				OutRotation);

			LHandTransform.SetLocation(OutPosition);
			LHandTransform.SetRotation(FQuat(OutRotation));
		}
	}
}

void UPartyAnimInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPartyAnimInstance, Speed);
	DOREPLIFETIME(UPartyAnimInstance, bIsInAir);
	DOREPLIFETIME(UPartyAnimInstance, bIsAccelerating);
}
