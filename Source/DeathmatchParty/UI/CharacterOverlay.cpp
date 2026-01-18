// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CharacterOverlay.h"

#include "Components/Image.h"

void UCharacterOverlay::PlayConnectionWarningAnimation()
{
	ConnectionWarning->SetRenderOpacity(1);
	PlayAnimation(BlinkConnectionWarning, 0, 5);
}

void UCharacterOverlay::StopConnectionWarningAnimation()
{
	ConnectionWarning->SetRenderOpacity(0);
	StopAnimation(BlinkConnectionWarning);
}
