#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "PartyTypes/Team.h"
#include "Weapon.generated.h"

class APartyPlayerController;
class APartyCharacter;
class UAnimationAsset;
class UTexture2D;
class USphereComponent;
class UWidgetComponent;
class USoundCue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponPickedUp);


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_Backup UMETA(DisplayName = "Backup"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX"),
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_MultipleHitScan UMETA(DisplayName = "Multiple Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Peojectile Weapon"),
	EFT_MAX UMETA(DisplayName = "DefaultMAX"),
};

UCLASS()
class DEATHMATCHPARTY_API AWeapon : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	ETeam WeaponTeam;

	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category="Weapon Properties")
	EWeaponState WeaponState;
	
	/**
	* Trace end with scatter
	*/

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;
	
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	UAnimationAsset* FireAnimationAsset;
	
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	float ZoomSpeed = 10.f;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	float AimSpread = 0.5f;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	float WeaponAmmo = 10.f;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	float MagazineCapacity = 30.f;
	
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	TSubclassOf<class ABulletShell> BulletShell;

	int32 AmmoSequence = 0;

	UFUNCTION()
	void SpendRound();
	
	UFUNCTION()
	void OnRep_WeaponState();

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);
	
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex);
	
public:	
	AWeapon();

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
	
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	float FireDelay = 0.2f;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	USoundCue* EquipSound;
		
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	UTexture2D* CrosshairCenter;
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	UTexture2D* CrosshairTop;
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	UTexture2D* CrosshairBottom;
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	UTexture2D* CrosshairLeft;
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	UTexture2D* CrosshairRight;
	
	virtual void Tick(float DeltaTime) override;
	virtual void OnRep_Owner() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Fire(const FVector& HitTarget);
	FVector_NetQuantize TraceEndWithScatter(const FVector& HitTarget);
	void MultipleTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets, uint32 NumberOfShots);

	virtual void Dropped();
	bool IsEmpty();
	void DisplayPickupWidget(bool bDisplayWidget) const;
	void SetWeaponState(EWeaponState State);
	void AddAmmo(int32 AmmoToAdd);
	void SetHUDAmmo();
	
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomSpeed() const { return ZoomSpeed; }
	FORCEINLINE float GetAimSpread() const { return AimSpread; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return WeaponAmmo; }
	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadshotDamage() const { return HeadshotDamage; }
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE ETeam GetWeaponTeam() const { return WeaponTeam; }

	void EnableCustomDepth(bool bEnable) const;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);
	
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnBackup();

	FOnWeaponPickedUp OnWeaponPickedUp;
	
protected:
	virtual void BeginPlay() override;

	virtual void OnWeaponStateSet();

	UPROPERTY()
	APartyCharacter* PartyCharacterOwner;

	UPROPERTY()
	APartyPlayerController* PartyControllerOwner;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	float Damage = 20.0f;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	float HeadshotDamage = 20.0f;

	UPROPERTY(Replicated, EditAnywhere, Category="Weapon Properties")
	bool bUseServerSideRewind = false;
};
