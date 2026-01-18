#pragma once

#include "CoreMinimal.h"
#include "Characters/PartyCharacter.h"
#include "Components/ActorComponent.h"
#include "Containers/CircularQueue.h"
#include "LagCompensationComponent.generated.h"

class APartyPlayerController;
class APartyCharacter;
class AWeapon;

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<APartyCharacter*, uint32> HeadShots;

	UPROPERTY()
	TMap<APartyCharacter*, uint32> BodyShots;
};

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	APartyCharacter* PartyCharacter;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DEATHMATCHPARTY_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 0.5f;

public:	
	friend class APartyCharacter;

	UPROPERTY()
	APartyCharacter* PartyCharacter;

	UPROPERTY()
	APartyPlayerController* PartyController;

protected:
	virtual void BeginPlay() override;
	void SaveFramePackage();

	void SaveFramePackage(FFramePackage& FramePackage);

	FFramePackage FrameInterpolation(
		const FFramePackage& OlderFrame,
		const FFramePackage& YoungerFrame,
		float HitTime);
	
	void CacheBoxPositions(
		const APartyCharacter* HitCharacter,
		FFramePackage& OutFramePackage);
	
	void MoveBoxes(
		const APartyCharacter* HitCharacter,
		const FFramePackage& Package);
	
	void ResetBoxes(
		const APartyCharacter* HitCharacter,
		const FFramePackage& Package);
	
	void EnableCharacterMeshCollision(
		const APartyCharacter* HitCharacter,
		ECollisionEnabled::Type CollisionEnabledType);
	
	FFramePackage GetFrameToCheck(
		APartyCharacter* HitCharacter,
		float HitTime);

	
	/** HIT SCAN WEAPON
	 * 
	 * @param HitCharacter	Character that has been hit
	 * @param TraceStart	Where the bullet starts
	 * @param HitLocation	Where the bullet hit something
	 * @param HitTime		The time where the bullet hit something
	 * @return				Result of the rewind check
	 */
	FServerSideRewindResult ServerSideRewind(
		APartyCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);

	/**
	 * @param Package		Frame to check
	 * @param HitCharacter	Character that has been hit
	 * @param TraceStart	Where the bullet starts
	 * @param HitLocation	Where the bullet hits something
	 * @return				Conformation of the hit on that frame
	 */
	FServerSideRewindResult ConfirmHit(
	const FFramePackage& Package,
	const APartyCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation);

	
	/** PROJECTILE WEAPON
	 * 
	 * @param HitCharacter	Character that has been hit
	 * @param TraceStart	Where the bullet starts
	 * @param InitVelocity	Initial velocity of the projectile
	 * @param HitTime		The time where the bullet hit something
	 * @return				Result of the rewind check
	 */
	FServerSideRewindResult ProjectileServerSideRewind(
		APartyCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitVelocity,
		float HitTime
	);

	/**
	 * @param Package 
	 * @param HitCharacter 
	 * @param TraceStart 
	 * @param InitVelocity 
	 * @param HitTime 
	 * @return 
	 */
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		const APartyCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitVelocity,
		float HitTime
	);
	
	
	/** SHOTGUN WEAPON
	 * 
	 * @param HitCharacters 
	 * @param TraceStart 
	 * @param HitLocations 
	 * @param HitTime 
	 * @return 
	 */
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<APartyCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);

	/**
	 * @param FramePackages 
	 * @param TraceStart 
	 * @param HitLocations 
	 * @return 
	 */
	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);
	
	
public:	
	ULagCompensationComponent();
	
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	void ShowFramePackage(
		const FFramePackage& FramePackage,
		const FColor Color) const;

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		APartyCharacter* HitCharacter,
		const FVector_NetQuantize TraceStart,
		const FVector_NetQuantize HitLocation,
		float HitTime);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<APartyCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);

	/**
	 * @param HitCharacter 
	 * @param TraceStart 
	 * @param InitVelocity 
	 * @param HitTime 
	 */
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		APartyCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitVelocity,
		float HitTime
	);
};
