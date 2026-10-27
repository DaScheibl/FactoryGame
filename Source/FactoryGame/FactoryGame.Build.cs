// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FactoryGame : ModuleRules
{
	public FactoryGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Engine-Module, gegen die das Spiel linkt: Kern (Core, Engine, InputCore), EnhancedInput für
		// die Steuerung, AIModule/StateTree für die Template-Varianten und UMG/Slate/SlateCore,
		// weil alle Menüs und das HUD in C++ aufgebaut werden.
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

		// Unterordner als Include-Pfade: dadurch reicht z. B. #include "FactoryInventoryComponent.h"
		// ohne "Items/" davor. Neue Unterordner hier eintragen.
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
