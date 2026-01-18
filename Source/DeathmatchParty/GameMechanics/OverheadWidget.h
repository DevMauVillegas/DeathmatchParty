#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

UCLASS()
class DEATHMATCHPARTY_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	UFUNCTION(BlueprintCallable)
	void SetDisplayText(FString inText);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* inPawn);

protected:
	virtual void NativeDestruct() override;
};
