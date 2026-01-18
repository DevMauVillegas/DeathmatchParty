#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PartyHUD.generated.h"

class APartyPlayerController;
class UTexture2D;
class UUserWidget;
class UCharacterOverlay;
class UWarmupAnnouncement;
class UElimAnnouncement;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

	float CrosshairSpread;
	FLinearColor CrosshairColor;
	
	UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
};

UCLASS()
class DEATHMATCHPARTY_API APartyHUD : public AHUD
{
	GENERATED_BODY()

	UPROPERTY()
	APartyPlayerController* PartyPlayerController;
	
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UElimAnnouncement> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 3.0f;

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimElements;
	
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* ElementToRemove); 
	
	
public:
	
	void AddCharacterOverlay();

	UPROPERTY(EditAnywhere, Category= "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	UPROPERTY(EditAnywhere, Category= "Player Stats")
	TSubclassOf<UUserWidget> WarmupWidgetClass;

	UPROPERTY()
	UWarmupAnnouncement* WarmupWidget;
	
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	void AddAnnouncement();

	void AddElimAnnouncement(const FString& AttackerName, const FString& VictimName);
	
	virtual void DrawHUD() override;
	
	FORCEINLINE void SetHUFPackage(const FHUDPackage& Package) { HUDPackage = Package; }

protected:
	virtual void BeginPlay() override;
	
};
