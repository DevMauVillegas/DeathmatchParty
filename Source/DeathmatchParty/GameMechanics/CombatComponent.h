#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/PartyHUD.h"
#include "Weapons/WeaponTypes.h"
#include "PartyTypes/CombatState.h"

#include "CombatComponent.generated.h"

class AWeapon;
class APartyCharacter;
class APartyPlayerController;
class APartyHUD;
class UCameraComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DEATHMATCHPARTY_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	
	bool bCanFire;
	bool bFireButtonPressed;

	// FOV when not aiming
	float DefaultFOV;
	float CurrentFOV;
	
	FHUDPackage HUDPackage;
	FTimerHandle FireTimer;
	
	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState;
	
	UPROPERTY()
	APartyCharacter* PartyCharacter;

	UPROPERTY()
	APartyPlayerController* PartyPlayerController;

	UPROPERTY()
	APartyHUD* HUD;
	
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* FollowCamera;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_BackupWeapon)
	AWeapon* BackupWeapon;
	
	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming;

	bool bAimButtonPressed;

	UFUNCTION()
	void OnRep_Aiming();
	
	bool bLocallyReloading;

	UPROPERTY(ReplicatedUsing=OnRep_HoldingTheFlag)
	bool bHoldingFlag;

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float BaseWalkSpeed;

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float AimWalkSpeed;

	UPROPERTY(EditAnywhere, Category="Combat")
	float ZoomedFOV;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	float ZoomInterpSpeed;
	
	UPROPERTY(EditDefaultsOnly, Category="Crosshair")
	float BaseSpread;

	UPROPERTY(EditDefaultsOnly, Category="Crosshair")
	float CrosshairInAirSpeed;
	
	UPROPERTY(EditDefaultsOnly, Category="Crosshair")
	float CrosshairInLandSpeed;

	UPROPERTY(EditDefaultsOnly, Category="Crosshair")
	float CrosshairAimSpeed;
	
	UPROPERTY(EditDefaultsOnly, Category="Crosshair")
	float CrosshairShootingSpeed;
	
	float CrosshairVelocityFactor;
	float CrosshairAimFactor;
	float CrosshairInAirFactor;
	float CrosshairShootingFactor;

	//
	// AMMO
	//
	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UPROPERTY(EditDefaultsOnly, Category="Ammo", meta=(ClampMin="0"))
	int32 StartingRifleAmmo;
	
	UPROPERTY(EditDefaultsOnly, Category="Ammo", meta=(ClampMin="0"))
	int32 StartingRocketAmmo;

	UPROPERTY(EditDefaultsOnly, Category="Ammo", meta=(ClampMin="0"))
	int32 StartingPistolAmmo;

	UPROPERTY(EditDefaultsOnly, Category="Ammo", meta=(ClampMin="0"))
	int32 StartingShotgunAmmo;

	UPROPERTY(EditDefaultsOnly, Category="Ammo", meta=(ClampMin="0"))
	int32 StartingSniperAmmo;

	UPROPERTY(EditDefaultsOnly, Category="Ammo", meta=(ClampMin="0"))
	int32 StartingGranadeAmmo;
	
	void StartFireTimer();
	void FireTimerFinished();
	void InterpFOV(float DeltaTime);
	bool CanFire();
	void InitializeCarriedAmmo();

	// Validates all needed variables were properly initialized with blueprints values
	UFUNCTION()
	void DefaultVariablesInitialization();
	
public:	
	UCombatComponent();

	void InitializePartyCharacter(APartyCharacter* NewPartyCharacter);

	friend class APartyCharacter;

	UFUNCTION(BlueprintCallable)
	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	int32 GetCarriedAmmo();

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentWeaponAmmo();

	UFUNCTION(BlueprintCallable)
	void Reload();
	
	void EquipWeapon(AWeapon* inWeapon);
	void PlayEquipWeaponSound(AWeapon* inWeapon);

	void SwapWeapon();

	void EquipPrimaryWeapon(AWeapon* inWeapon);
	void EquipBackupWeapon(AWeapon* inWeapon);
	
	void AttachPrimaryWeapon(AWeapon* inWeapon) const;
	void AttachBackupWeapon(AWeapon* inWeapon) const;
	void AttachFlagToLeftHand(AWeapon* inWeapon) const;

	bool ShouldSwapWeapons() const;

	int32 AmountToReload();
	void PickUpAmmo(EWeaponType WeaponType, int32 AmmoAmount);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishSwapping();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttach();
	
protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	void Fire();
	void FireProjectileWeapon(const FVector_NetQuantize& TraceHitTarget);
	void FireHitScanWeapon(const FVector_NetQuantize& TraceHitTarget);
	void FireMultipleHitScanWeapon(const FVector_NetQuantize& TraceHitTarget);

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	void SetHUDCrosshair(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_CarriedAmmo();
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_BackupWeapon();

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION()
	void OnRep_HoldingTheFlag();

	UPROPERTY()
	AWeapon* TheFlag;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();
	void UpdateCarriedAmmo();
	void UpdateAmmoValues();
};
