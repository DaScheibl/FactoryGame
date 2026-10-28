// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactoryItemTypes.h"
#include "FactoryBuildComponent.generated.h"

class AFactoryGamePlayerController;
class AFactoryHologram;
class AActor;
struct FFactoryBuildingDefinition;

/** What the build tool is doing */
enum class EFactoryBuildMode : uint8
{
	/** Not building; looking at a demolition crate shows "E: take items" */
	None,

	/** A building from the hotbar follows the crosshair as a hologram */
	Place,

	/** Holding the primary button on a building demolishes it */
	Demolish
};

/**
 *  The build tool of the player: placing buildings, demolishing them and emptying demolition crates.
 *  Lives on the AFactoryGamePlayerController, which forwards the build keys to it.
 *
 *  Ablauf Platzieren: Hotbar-Taste wählt ein Gebäude → Hologramm folgt dem Fadenkreuz (1-m-Raster,
 *  Drehen mit dem Mausrad) → grün, wenn der Boden eben genug ist (max. 1 m Höhenunterschied, das Gebäude
 *  sinkt dann ein), nichts im Weg steht und das Material reicht → Linksklick baut und zieht die Kosten ab.
 *  Ablauf Abriss: F → Ziel leuchtet rot → Linksklick 0,5 s halten → alles zurück ins Inventar,
 *  was nicht passt, landet in einer Abriss-Kiste.
 *  Regeln und Steuerung: Docs/Standards/13-bauen.md
 */
UCLASS(ClassGroup=(Factory))
class FACTORYGAME_API UFactoryBuildComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** How far the player can build and demolish, in cm */
	static constexpr double BuildRange = 4000.0;

	/** How far a demolition crate can be emptied from, in cm */
	static constexpr double InteractRange = 800.0;

	/** Highest ground difference under a building, in cm. The building sinks into the higher ground */
	static constexpr double MaxGroundDifference = 100.0;

	/** How long the primary button has to be held to demolish, in seconds */
	static constexpr float DemolishHoldTime = 0.5f;

	UFactoryBuildComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	EFactoryBuildMode GetMode() const { return Mode; }

	/** Starts placing a building. Unknown ids stop building */
	void BeginPlacing(FName BuildingId);

	/** Switches the demolish mode on or off */
	void ToggleDemolish();

	/** Leaves place or demolish mode */
	void StopBuilding();

	/** Primary button (left mouse, RT) pressed or released */
	void PrimaryPressed();
	void PrimaryReleased();

	/** Rotates the hologram by one step. Fine steps (15°) instead of 90° */
	void Rotate(int32 Direction, bool bFine);

	/** Interact key (E): empties the demolition crate in view */
	void Interact();

protected:

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	AFactoryGamePlayerController* GetPlayerController() const;

	/** Line trace along the view, ignoring the player and the hologram */
	bool TraceView(double Range, FHitResult& OutHit) const;

	/** Moves the hologram to the crosshair and checks if it can be built there */
	void UpdatePlacement();

	/** Finds the demolish target and advances the hold timer */
	void UpdateDemolish(float DeltaTime);

	/** Shows the crate prompt when looking at a demolition crate */
	void UpdateIdle();

	/** Builds the current placement. Returns false if it isn't valid */
	bool PlaceBuilding();

	/** Demolishes a machine or belt and gives everything back */
	void Demolish(AActor* Target);

	/** Highlights the demolish target (and removes the highlight from the old one) */
	void SetDemolishTarget(AActor* NewTarget);

	/** Returns the cost lines for the HUD and whether the player has everything */
	bool CheckCosts(const TArray<FFactoryItemStack>& Costs, TArray<TPair<FText, bool>>& OutLines) const;

	/** Sends the build panel to the HUD */
	void ShowPanel(const FText& Title, const TArray<TPair<FText, bool>>& Lines, const FText& Error, const FText& Controls) const;

	void HidePanel() const;

	EFactoryBuildMode Mode = EFactoryBuildMode::None;

	/** Building being placed */
	FName BuildingId;

	/** Rotation of the hologram around the up axis */
	double PlaceYaw = 0.0;

	/** Current hologram transform and whether it can be built there */
	FTransform PlaceTransform;
	bool bPlaceValid = false;

	UPROPERTY()
	TObjectPtr<AFactoryHologram> Hologram;

	UPROPERTY()
	TWeakObjectPtr<AActor> DemolishTarget;

	/** True while the primary button is held in demolish mode */
	bool bHoldingPrimary = false;

	float HoldTime = 0.0f;
};
