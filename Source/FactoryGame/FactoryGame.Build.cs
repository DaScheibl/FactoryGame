// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FactoryGame : ModuleRules
{
	public FactoryGame(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"FactoryGame",
			"FactoryGame/Menu",
			"FactoryGame/Items",
			"FactoryGame/UI",
			"FactoryGame/Variant_Horror",
			"FactoryGame/Variant_Horror/UI",
			"FactoryGame/Variant_Shooter",
			"FactoryGame/Variant_Shooter/AI",
			"FactoryGame/Variant_Shooter/UI",
			"FactoryGame/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
