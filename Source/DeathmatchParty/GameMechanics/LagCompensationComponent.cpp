
#include "GameMechanics/LagCompensationComponent.h"

#include "DeathmatchParty.h"
#include "ProjectDescriptor.h"
#include "Characters/PartyCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/Weapon.h"

ULagCompensationComponent::ULagCompensationComponent() :
	PartyCharacter(nullptr),
	PartyController(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULagCompensationComponent::SaveFramePackage()
{
	FFramePackage ThisFrame;
	SaveFramePackage(ThisFrame);
	FrameHistory.AddHead(ThisFrame);

	float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;

	while (HistoryLength > MaxRecordTime)
	{
		FrameHistory.RemoveNode(FrameHistory.GetTail());
		HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
	}
}

void ULagCompensationComponent::TickComponent(
	float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PartyCharacter != nullptr && PartyCharacter->HasAuthority())
	{
		SaveFramePackage();
	}
	
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& FramePackage)
{
	PartyCharacter = PartyCharacter == nullptr ? Cast<APartyCharacter>(GetOwner()) : PartyCharacter;

	if (PartyCharacter)
	{
		FramePackage.Time = GetWorld()->GetTimeSeconds();
		FramePackage.PartyCharacter = PartyCharacter;
		for (auto& BoxPair : PartyCharacter->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			
			FramePackage.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}

FFramePackage ULagCompensationComponent::FrameInterpolation(
	const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame,
	float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpolationFraction = FMath::Clamp((HitTime - OlderFrame.Time) - Distance, 0.0f, 1.0f);

	FFramePackage InterpolatedFrame;

	InterpolatedFrame.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;

		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpolatedBoxInfo;
		InterpolatedBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.0f, InterpolationFraction);
		InterpolatedBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.0f, InterpolationFraction);
		InterpolatedBoxInfo.BoxExtent = YoungerBox.BoxExtent;
		
		InterpolatedFrame.HitBoxInfo.Add(BoxInfoName, InterpolatedBoxInfo);
	}

	return InterpolatedFrame;
}

void ULagCompensationComponent::CacheBoxPositions(
	const APartyCharacter* HitCharacter,
	FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr)
	{
		return;
	}

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
			
		}
	}
}

void ULagCompensationComponent::MoveBoxes(
	const APartyCharacter* HitCharacter,
	const FFramePackage& Package)
{
	if (HitCharacter == nullptr)
	{
		return;
	}

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
		HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
		HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
	}
}

void ULagCompensationComponent::ResetBoxes(
	const APartyCharacter* HitCharacter,
	const FFramePackage& Package)
{
	if (HitCharacter == nullptr)
	{
		return;
	}

	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
		HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
		HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(
	const APartyCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabledType)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabledType);
	}
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(
	APartyCharacter* HitCharacter,
	const float HitTime)
{
	bool bReturn =
	HitCharacter == nullptr ||
	!HitCharacter->GetLagCompensationComponent() ||
	!HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() ||
	!HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail();

	if (bReturn)
	{
		return FFramePackage();
	}
	
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;

	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	
	if (OldestHistoryTime > HitTime)
	{
		return FFramePackage();
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* OlderFrame = History.GetHead();

	for (auto It = begin(History); It; ++It)
	{
		const auto CurrentFrame = *It;
		if (CurrentFrame.Time <= HitTime)
		{
			OlderFrame = It.GetNode();
		}
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* YoungerFrame = OlderFrame->GetPrevNode();
	
	FFramePackage FrameToCheck = FrameInterpolation(OlderFrame->GetValue(), YoungerFrame->GetValue(), HitTime);
	FrameToCheck.PartyCharacter = HitCharacter;

	return FrameToCheck;
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(
	APartyCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize100& InitVelocity,
	float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitVelocity, HitTime);
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(
	const FFramePackage& Package,
	const APartyCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr)
	{
		return FServerSideRewindResult();
	}

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];

	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

	bool bIsHeadshot = false;
	
	if (UWorld* World = GetWorld())
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);

		if (ConfirmHitResult.bBlockingHit)
		{
			bIsHeadshot = true;
		}
		
		else
		{
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
				}
			}

			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
		}
	}
	
	ResetBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{static_cast<bool>(ConfirmHitResult.bBlockingHit), bIsHeadshot};
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(
	const FFramePackage& Package,
	const APartyCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize100& InitVelocity,
	float HitTime)
{
	if (HitCharacter == nullptr)
	{
		return FServerSideRewindResult();
	}

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];

	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);

	FPredictProjectilePathParams PathParams;
	FPredictProjectilePathResult PathResult;
	
	PathParams.bTraceWithChannel = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.0f;
	PathParams.ProjectileRadius = 5.0f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	bool WasHit = false;
	bool WasHeadShot = false;

	if (PathResult.HitResult.bBlockingHit)
	{
		WasHit = true;
		WasHeadShot = true;
	}
	
	else
	{
		for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			}
		}
		
		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

		if (PathResult.HitResult.bBlockingHit)
		{
			WasHit = true;
		}
	}
	
	ResetBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	
	return FServerSideRewindResult{WasHit, WasHeadShot};
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(APartyCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitVelocity, float HitTime)
{
	const FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitVelocity, HitTime);

	if (HitCharacter && PartyCharacter && PartyCharacter->GetEquippedWeapon() && Confirm.bHitConfirmed)
	{
		AWeapon* EquippedWeapon = PartyCharacter->GetEquippedWeapon();
		const float Damage = Confirm.bHeadShot ? EquippedWeapon->GetHeadshotDamage() : EquippedWeapon->GetDamage();
		
		UGameplayStatics::ApplyDamage(HitCharacter,
			Damage,
			PartyCharacter->GetController(),
			EquippedWeapon,
			UDamageType::StaticClass()
		);
	}	
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(
	const TArray<APartyCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations,
	float HitTime)
{
	TArray<FFramePackage> FramesToCheck;

	for (APartyCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}

	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(
	const TArray<FFramePackage>& FramePackages,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations)
{
	FShotgunServerSideRewindResult ShotgunResult;

	if (GetWorld() == nullptr)
	{
		return FShotgunServerSideRewindResult();
	}

	TArray<FFramePackage> CurrentFrames;
	for (auto& Frame : FramePackages)
	{
		if (Frame.PartyCharacter == nullptr)
		{
			continue;
		}
		
		FFramePackage CurrentFrame;
		CurrentFrame.PartyCharacter = Frame.PartyCharacter;
		CacheBoxPositions(Frame.PartyCharacter, CurrentFrame);
		MoveBoxes(Frame.PartyCharacter, Frame);
		EnableCharacterMeshCollision(Frame.PartyCharacter, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);

		UBoxComponent* HeadBox = Frame.PartyCharacter->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
	}

	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

		GetWorld()->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);

		if (APartyCharacter* HitPartyCharacter = Cast<APartyCharacter>(ConfirmHitResult.GetActor()))
		{
			if (ShotgunResult.HeadShots.Contains(HitPartyCharacter))
			{
				ShotgunResult.HeadShots[HitPartyCharacter]++;
			}
			else
			{
				ShotgunResult.HeadShots.Emplace(HitPartyCharacter, 1);
			}
		}
	}

	for (auto& Frame : FramePackages)
	{
		if (Frame.PartyCharacter == nullptr)
		{
			continue;
		}
		
		for (auto& HitBoxPair : Frame.PartyCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			}
		}

		UBoxComponent* HeadBox = Frame.PartyCharacter->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

		GetWorld()->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);

		if (APartyCharacter* HitPartyCharacter = Cast<APartyCharacter>(ConfirmHitResult.GetActor()))
		{
			if (ShotgunResult.BodyShots.Contains(HitPartyCharacter))
			{
				ShotgunResult.BodyShots[HitPartyCharacter]++;
			}
			else
			{
				ShotgunResult.BodyShots.Emplace(HitPartyCharacter, 1);
			}
		}
	}

	for (auto& Frame : CurrentFrames)
	{
		ResetBoxes(Frame.PartyCharacter, Frame);
		EnableCharacterMeshCollision(Frame.PartyCharacter, ECollisionEnabled::QueryAndPhysics);
	}

	return ShotgunResult;
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(
	APartyCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation,
	const float HitTime)
{
	const FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(
	const TArray<APartyCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations,
	float HitTime)
{
	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for (auto HitCharacter : HitCharacters)
	{
		float TotalDamage = 0.0f;
		if (HitCharacter  && HitCharacter->GetEquippedWeapon())
		{
			AWeapon* EquippedWeapon = HitCharacter->GetEquippedWeapon(); 
			TotalDamage += Confirm.HeadShots[HitCharacter] * EquippedWeapon->GetDamage();
			TotalDamage += Confirm.BodyShots[HitCharacter] * EquippedWeapon->GetHeadshotDamage();
		}

		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			PartyCharacter->GetController(),
			PartyCharacter->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(
	APartyCharacter* HitCharacter,
	const FVector_NetQuantize TraceStart,
	const FVector_NetQuantize HitLocation,
	const float HitTime)
{
	const FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (HitCharacter && PartyCharacter && PartyCharacter->GetEquippedWeapon() && Confirm.bHitConfirmed)
	{
		AWeapon* EquippedWeapon = PartyCharacter->GetEquippedWeapon();
		const float Damage = Confirm.bHeadShot ? EquippedWeapon->GetHeadshotDamage() : EquippedWeapon->GetDamage();
		
		UGameplayStatics::ApplyDamage(HitCharacter,
			Damage,
			PartyCharacter->GetController(),
			EquippedWeapon,
			UDamageType::StaticClass()
		);
	}
}

