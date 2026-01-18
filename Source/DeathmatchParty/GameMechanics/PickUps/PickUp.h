#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUp.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class USphereComponent;
class USoundCue;

UCLASS()
class DEATHMATCHPARTY_API APickUp : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere, Category="PickUpSettings")
	USoundCue* PickUpSound;

	UPROPERTY(EditAnywhere, Category="PickUpSettings")
	UStaticMeshComponent* PickUpMesh;

	FTimerHandle BindOverlapHandle;

	float BindOverlapTime = 0.2;

	UFUNCTION()
	void BindOverlapTimeFinish();
	
public:	
	APickUp();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	UPROPERTY(VisibleAnywhere, Category="PickUpSettings")
	UNiagaraComponent* PickUpEffectComponent;

	UPROPERTY(EditAnywhere, Category="PickUpSettings")
	UNiagaraSystem* PickUpEffect;

protected:

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.0f;
	
	UPROPERTY()
	FVector OverlappingActorLocation;
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
