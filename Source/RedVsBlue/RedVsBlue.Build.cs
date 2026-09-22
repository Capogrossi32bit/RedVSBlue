// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RedVsBlue : ModuleRules
{
	public RedVsBlue(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"RedVsBlue",
			"RedVsBlue/Variant_Platforming",
			"RedVsBlue/Variant_Platforming/Animation",
			"RedVsBlue/Variant_Combat",
			"RedVsBlue/Variant_Combat/AI",
			"RedVsBlue/Variant_Combat/Animation",
			"RedVsBlue/Variant_Combat/Gameplay",
			"RedVsBlue/Variant_Combat/Interfaces",
			"RedVsBlue/Variant_Combat/UI",
			"RedVsBlue/Variant_SideScrolling",
			"RedVsBlue/Variant_SideScrolling/AI",
			"RedVsBlue/Variant_SideScrolling/Gameplay",
			"RedVsBlue/Variant_SideScrolling/Interfaces",
			"RedVsBlue/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
