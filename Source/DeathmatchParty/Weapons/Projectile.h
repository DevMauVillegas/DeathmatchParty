#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;
class UParticleSystem;
class UParticleSystemComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundCue;

UCLASS()
class DEATHMATCHPARTY_API AProjectile : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Projectile Settings")
	UParticleSystem* Tracer;

	UParticleSystemComponent* TracerComponent;
	
public:	
	AProjectile();
	
	virtual void Destroyed() override;
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;
	
	UPROPERTY(EditAnywhere, Category="Projectile Settings")
	float DestroyTime = 3.0f;

	UPROPERTY(EditAnywhere, Category="Projectile Settings")
	float Damage = 15.f;
	
	UPROPERTY(EditAnywhere, Category="Projectile Settings")
	float HeadShotDamage = 25.f;
	
	FTimerHandle DestroyTimer;

	/**
	* Used with server side rewind
	*/

	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float ProjectileInitialSpeed = 15000;

protected:
	
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;
	
	UPROPERTY(EditAnywhere, Category="Projectile Settings")
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, Category="Projectile Settings")
	USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere, Category="Projectile Settings")
	UNiagaraSystem* TrailSystem;

	UPROPERTY(EditAnywhere, Category="Projectile Settings")
	float DamageInnerRadius = 200;

	UPROPERTY(EditAnywhere, Category="Projectile Settings")
	float DamageOuterRadius = 500;

	UPROPERTY(EditAnywhere, Category="Projectile Settings")
	UBoxComponent* CollisionBox;

	UPROPERTY()
	UNiagaraComponent* NiagaraComponent;
	
	virtual void BeginPlay() override;

	void StartDestroyTimer();
	void DestroyTimerFinished();
	void SpawnTrailSystem();
	void ExplodeDamage();

	UFUNCTION()
	virtual void OnHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit
	);
};
