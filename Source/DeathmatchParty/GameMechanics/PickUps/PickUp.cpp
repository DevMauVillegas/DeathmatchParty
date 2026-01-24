
#include "GameMechanics/PickUps/PickUp.h"

#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "Weapons/WeaponTypes.h"


APickUp::APickUp()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(150.0f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);\
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	OverlapSphere->AddLocalOffset(FVector(0.0f, 0.0f, 85.0f));
	
	PickUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickUpMesh"));
	PickUpMesh->SetupAttachment(RootComponent);
	PickUpMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickUpMesh->SetRelativeScale3D(FVector(5.0, 5.0f, 5.0f));
	PickUpMesh->SetRenderCustomDepth(true);
	PickUpMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);

	PickUpEffectComponent = CreateDefaultSubobject<UNiagaraComponent>("PickUpEffectComponent");
	PickUpEffectComponent->SetupAttachment(RootComponent);

	BindOverlapTime = 0.2;
}

void APickUp::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorldTimerManager().SetTimer(BindOverlapHandle, this, &APickUp::BindOverlapTimeFinish, BindOverlapTime);
}

void APickUp::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void APickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PickUpMesh)
	{
		PickUpMesh->AddLocalRotation(FRotator(0.0f, BaseTurnRate * DeltaTime, 0.0f));
	}
}

void APickUp::Destroyed()
{
	if (PickUpSound == nullptr)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		PickUpSound,
		GetActorLocation());
	
	Super::Destroyed();
}

void APickUp::BindOverlapTimeFinish()
{
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
}
