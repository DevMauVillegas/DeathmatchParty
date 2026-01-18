#pragma once

#define TRACE_LENGHT 120000.f
#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_CYAN 252

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_Shotgun UMETA(DisplayName = "Shutgun"),
	EWT_Sniper UMETA(DisplayName = "Sniper"),
	EWT_GranadeLauncher UMETA(DisplayName = "GranadeLauncher"),
	EWT_Flag UMETA(DisplayName = "Flag"),
	EWT_MAX UMETA(DisplayName = "DefaultMAX", Hidden)
};