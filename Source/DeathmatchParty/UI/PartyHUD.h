#pragma once

#include "CoreMinimal.h"
#include "PartyHUDInterface.h"
#include "GameFramework/HUD.h"
#include "GameState/PartyGameState.h"
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

	FHUDPackage()
	{
		CrosshairSpread = 0.0f;
		CrosshairColor = FLinearColor::Black;
		CrosshairCenter = nullptr;
		CrosshairTop = nullptr;
		CrosshairBottom = nullptr;
		CrosshairLeft = nullptr;
		CrosshairRight = nullptr;
	}

	float CrosshairSpread;
	FLinearColor CrosshairColor;

	UPROPERTY()
	UTexture2D* CrosshairCenter;
	
	UPROPERTY()
	UTexture2D* CrosshairTop;

	UPROPERTY()
	UTexture2D* CrosshairBottom;

	UPROPERTY()
	UTexture2D* CrosshairLeft;

	UPROPERTY()
	UTexture2D* CrosshairRight;
};

UCLASS()
class DEATHMATCHPARTY_API APartyHUD : public AHUD, public IPartyHUDInterface
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

	virtual void SetShield(float Current, float Max) override;
	virtual void SetHealth(float Current, float Max) override;
	virtual void SetScore(const int32 NewScore) override;
	virtual void SetDefeats(const int32 NewDefeats) override;
	virtual void SetWeaponAmmo(const int32 NewWeaponAmmo) override;
	virtual void SetCarriedAmmo(const int32 NewCarriedAmmo) override;

	virtual void SetRedTeamScore(const int32 NewRedTeamScore) override;
	virtual void SetBlueTeamScore(const int32 NewBlueTeamScore) override;
	virtual void HideTeamUIElements() override;

	virtual void SetAnnouncementCountdown(const float NewAnnouncementCountdownTime) override;
	virtual void SetMatchTime(const float NewTimerValue) override;

	virtual void HandleCooldown() override;

	virtual void DisplayConnectionWarning(const bool bDisplayConnectionWarning) override;

	void ShowTeamScoresUpdated(const bool bShowTeamScores);
	
	void AddCharacterOverlay();

	FString GetTeamInfoText(const APartyGameState* PartyGameState);
	FString GetInfoText(const TArray<APartyPlayerState*>& PlayerStates);

	UPROPERTY(EditAnywhere, Category= "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	UPROPERTY(EditAnywhere, Category= "Player Stats")
	TSubclassOf<UUserWidget> WarmupWidgetClass;

	UPROPERTY()
	UWarmupAnnouncement* WarmupWidget;
	
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	bool bShowTeamScores;

	void AddAnnouncement();

	void AddElimAnnouncement(const FString& AttackerName, const FString& VictimName);
	
	virtual void DrawHUD() override;
	
	FORCEINLINE void SetHUFPackage(const FHUDPackage& Package) { HUDPackage = Package; }

protected:
	virtual void BeginPlay() override;
	
};
