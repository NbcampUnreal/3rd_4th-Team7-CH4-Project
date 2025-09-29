// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TreasureHunter : ModuleRules
{
	public TreasureHunter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"GameplayAbilities", 
			"GameplayTags", 
			"GameplayTasks",
            "OnlineSubsystem"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate",
            "SlateCore",
            "UMG",
            "AnimGraphRuntime",
            "MotionWarping",
            "Niagara",
            "OnlineSubsystemUtils",
            "AdvancedSessions",
            "AdvancedSteamSessions"
        });

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
        }

        PublicIncludePaths.AddRange(new string[] { "TreasureHunter" });

	}
}
