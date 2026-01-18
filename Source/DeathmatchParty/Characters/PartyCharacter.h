#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "PartyAttributeSet.h"
#include "PartyTypes/TurningInPlace.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "PartyTypes/Team.h"
#include "Interfaces/InteractWithCrosshairInterface.h"
#include "PartyTypes/CombatState.h"
#include "PartyCharacter.generated.h"

class UAttributeSet;
class UGameplayEffect;
class UPartyAttributeSet;
class UAbilitySystemComponent;
class APartyGameMode;
class UNiagaraSystem;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

class APartyPlayerState;
class UCombatComponent;
class UBuffComponent;
class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class AWeapon;
class UAnimMontage;
class APartyPlayerController;
class AController;
class UOverheadWidget;
class UBoxComponent;
class ULagCompensationComponent;
class UNiagaraComponent;

UCLASS()
class DEATHMATCHPARTY_API APartyCharacter : public ACharacter, public IInteractWithCrosshairInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

	
	/**
	 * Character Elements and subobjects
	 */

	UPROPERTY(VisibleAnywhere, Category=Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverhearWidget;

	UPROPERTY(VisibleAnywhere, Category = "Abilities")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	const UPartyAttributeSet* AttributeSet;

	/**
	 * End of character elements and subobjects
	 */

	
	FString PlayerName;
	bool bIsAiming;
	bool bRotateRootBone;
	
	float AO_Yaw;
	float AO_Pitch;
	float InterpAO_Yaw;
	float ProxiYaw;
	float TurnThreshold = 3.f;
	float TimeSinceLastMovementReplication;
	
	FRotator StartingAimRotation;
	FRotator ProxiRotatorLastFrame;
	FRotator ProxiRotation;

	ETurningInPlace TurningInPlace;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	
	FOnTimelineFloat DissolveTrack;

	UPROPERTY()
	APartyPlayerController* PartyPlayerController;
	
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere)
	UBuffComponent* BuffComponent;

	UPROPERTY(VisibleAnywhere)
	ULagCompensationComponent* LagCompensationComponent;
	
	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* HitReactionMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* SwapWeaponMontage;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeapon;

	void TurnInPlace(float DeltaTime);
	void HideCharacterIfCharacterClose() const;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	
	// Attribute change callback
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	// Attribute change callback
	void OnShieldChanged(const FOnAttributeChangeData& Data);

	// Bind the delegate
	void BindAttributeChangeDelegates();
	
	UFUNCTION()
	void OnRep_OverlappingWeapon(const AWeapon* LastWeapon) const;

	UFUNCTION(Server, Reliable)
	void ServerEquippedButtonPressed();

	void SetupCollisionBoxes();
	
public:
	
	APartyCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> ShieldDamageEffect;
	
	/**
	 *  Hit boxes used for server-side rewind region
	 */
#pragma region CollisionBoxes
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* HeadBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* PelvisBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine_02Box;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine_03Box;

	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArm_1Box;

	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArm_2Box;

	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArm_1Box;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArm_2Box;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Hand_1Box;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Hand_2Box;

	UPROPERTY(EditAnywhere)
	UBoxComponent* BackpackBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* BlanketBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperLeg_1Box;

	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperLeg_2Box;

	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerLeg_1Box;

	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerLeg_2Box;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Foot_1Box;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Foot_2Box;

	TMap<FName, UBoxComponent*> HitCollisionBoxes;
	
	/**
	* End of the insanity of the box
	*/

	#pragma endregion CollisionBoxes


	/**
	 * Team Colors
	 */

	UFUNCTION()
	void SetTeamColor(ETeam TeamSelected);

	UPROPERTY(EditAnywhere, Category=Elim)
	UMaterialInstance* OriginalMaterialInstance;
	
	UPROPERTY(EditAnywhere, Category=Elim)
	UMaterialInstance* RedMaterialInstance;
	
	UPROPERTY(EditAnywhere, Category=Elim)
	UMaterialInstance* RedDissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category=Elim)
	UMaterialInstance* BlueMaterialInstance;
	
	UPROPERTY(EditAnywhere, Category=Elim)
	UMaterialInstance* BlueDissolveMaterialInstance;
	
	/**
	 * Elimination and exit region
	 */
#pragma region EliminationAndExit
	
	void Eliminated(bool bPlayerLeftGame);
	void PlayEliminatedMontage() const;
	void EliminatedTimerFinished();
	void StartDissolve();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated(bool bPlayerLeftGame);

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	FORCEINLINE bool IsEliminated() const { return bIsEliminated; }
	
	UPROPERTY(VisibleAnywhere, Category=Elimination)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category=Elimination)
	UMaterialInstance* DissolvedMaterialInstance;
	
	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* EliminatedMontage;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
	
	bool bLeftGame = false;

	bool bIsEliminated;
	FTimerHandle EliminatedTimer;
	
	FOnLeftGame OnLeftGame;
	
	/**
	 * Elimination Section End
	 */
	
	#pragma endregion EliminationAndExit


	/**
	 * Crown effect and such
	 */
#pragma region CrownEffect

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* CrownSystem;

	UPROPERTY()
	UNiagaraComponent* CrownComponent;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();
	
#pragma endregion
	
	UPROPERTY(Replicated)
	bool bDisabledGameplay = false;
	
	UPROPERTY()
	APartyPlayerState* PartyPlayerState;

	UPROPERTY()
	APartyGameMode* PartyGameMode;

	void SetSpawnPoint();
	
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_ReplicatedMovement() override;
	
	void SetOverlappingWeapon(AWeapon* inWeapon);
	void PlayFireMontage(bool bIsAiming) const;
	void PlayHitReactionMontage() const;
	void PlayReloadMontage() const;
	void PlaySwapMontage() const;
	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyWeapons();
	bool IsAiming() const;
	bool IsWeaponEquipped() const;
	bool IsHoldingTheFlag() const;
	
	AWeapon* GetEquippedWeapon() const;
	ECombatState GetCombatState() const;
	
	FORCEINLINE bool ShouldRotateRootBone() { return bRotateRootBone; }
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() { return FollowCamera; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return LagCompensationComponent; }
	FORCEINLINE const UPartyAttributeSet* GetAttributeSet() const { return AttributeSet; }

	void SetHoldingTheFlag(bool bIsHolding);
	void ApplyHealthDamageEffect(float DamageToHealth);
	void ApplyShieldDamageEffect(float DamageToShield);

	UFUNCTION()
	virtual void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void SpawnDefaultWeapon();

	ETeam GetTeam();
	
protected:
	
	void RotateInPlace(float DeltaTime);
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveCameraX(float Value);
	void MoveCameraY(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed() ;
	void FireButtonPressed() ;
	void FireButtonReleased() ;
	void ReloadButtonPressed() ;
	void CalculateAO_Pitch();
	float CalculateSpeed() const;

	virtual void Destroyed() override;
	virtual void BeginPlay() override;
	virtual void Jump() override;

	void AimOffset(float DeltaTime);

	void SimProxisTurn();
	void PollInit();
};
