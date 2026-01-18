// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ElimAnnouncement.h"

#include "Components/TextBlock.h"

void UElimAnnouncement::SetElimAnnouncementText(const FString& AttackerName, const FString& VictimName) const
{
	const FString NewElimText = FString::Printf(TEXT("%s killed %s"), *AttackerName, *VictimName);
	if (ElimPlayerText)
	{
		ElimPlayerText->SetText(FText::FromString(NewElimText));
	}
}
