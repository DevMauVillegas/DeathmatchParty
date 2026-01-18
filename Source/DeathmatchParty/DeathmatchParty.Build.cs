// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DeathmatchParty : ModuleRules
{
	public DeathmatchParty(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(new string[] { "DeathmatchParty" });
		PrivateIncludePaths.AddRange(new string[] { "DeathmatchParty" });
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"Niagara",
			"OnlineSubsystem", 
			"OnlineSubsystemUtils", 
			"OnlineSubsystemSteam", 
			"MultiplayerSessions",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks" 
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
