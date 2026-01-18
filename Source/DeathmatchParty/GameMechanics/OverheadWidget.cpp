#include "GameMechanics/OverheadWidget.h"

#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText(FString inText)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(inText));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* inPawn)
{
	const ENetRole RemoteRole = inPawn->GetRemoteRole();

	FString StringRemoteRole;
	switch (RemoteRole)
	{
	case ENetRole::ROLE_Authority:
		StringRemoteRole = "Authority";
		break;
	case ENetRole::ROLE_AutonomousProxy:
		StringRemoteRole = "Autonomous Proxy";
		break;
	case ENetRole::ROLE_SimulatedProxy:
		StringRemoteRole = "Simulated Proxy";
		break;
	default:
		StringRemoteRole = "None";
	}
	
	SetDisplayText(StringRemoteRole);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	
	Super::NativeDestruct();
}
