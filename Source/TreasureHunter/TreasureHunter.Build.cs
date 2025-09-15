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
			"AnimGraphRuntime", 
			"GameplayAbilities", 
			"GameplayTags", 
			"GameplayTasks",
			"UMG",
			"Slate",
			"SlateCore",
			"MotionWarping"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
		
		PublicIncludePaths.AddRange(new string[] { "TreasureHunter" });

	}
}
