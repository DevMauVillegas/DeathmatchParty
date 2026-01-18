#include "Weapons/BulletShell.h"

#include "Kismet/GameplayStatics.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "Sound/SoundCue.h"

ABulletShell::ABulletShell()
{
	PrimaryActorTick.bCanEverTick = false;

	BulletShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletShellMesh"));
	SetRootComponent(BulletShellMesh);

	BulletShellMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	BulletShellMesh->SetSimulatePhysics(true);
	BulletShellMesh->SetEnableGravity(true);
	BulletShellMesh->SetNotifyRigidBodyCollision(true);

	ShellEjectionImpulse = 10.f;
}

void ABulletShell::BeginPlay()
{
	Super::BeginPlay();
	BulletShellMesh->OnComponentHit.AddDynamic(this, &ABulletShell::OnHit);
	BulletShellMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);

	SetLifeSpan(3.f);
}

void ABulletShell::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	
	BulletShellMesh->SetNotifyRigidBodyCollision(false);
}
