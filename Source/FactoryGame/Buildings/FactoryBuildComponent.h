// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactoryItemTypes.h"
#include "FactoryBeltPath.h"
#include "FactoryBuildComponent.generated.h"

class AFactoryGamePlayerController;
class AFactoryHologram;
class AFactoryMachine;
class AFactoryBeltPole;
class AFactoryResourceNode;
class AActor;
struct FFactoryBuildingDefinition;

/** What the build tool is doing */
enum class EFactoryBuildMode : uint8
{
	/** Not building; looking at a demolition crate shows "E: take items" */
	None,

	/** A machine from the hotbar follows the crosshair as a hologram */
	Place,

	/** A conveyor pole follows the crosshair, the mouse wheel sets its height */
	Pole,

	/** Drawing a belt: first click picks the start, second click the end */
	Belt,

	/** Foundations and ramps: click places one, holding and dragging places a row or an area */
	Foundation,

	/** Holding the primary button on a building demolishes it */
	Demolish
};

/** Which side of a belt FindBeltEnd looks for */
enum class EFactoryBeltEndWant : uint8
{
	/** Where a belt can begin: free machine output, free pole output */
	Output,

	/** Where a belt can end: free machine input, free pole input */
	Input,

	/** First click: either, the belt is then drawn forward or backward */
	Either
};

/** One end of a belt that is being drawn: a machine port, an existing pole or a new pole on the ground */
struct FFactoryBeltEnd
{
	TWeakObjectPtr<AFactoryMachine> Machine;
	int32 Port = INDEX_NONE;

	TWeakObjectPtr<AFactoryBeltPole> Pole;

	/** A pole is built here together with the belt */
	bool bNewPole = false;
	FVector PoleFoot = FVector::ZeroVector;
	int32 PoleHeight = 1;

	/** Belt connection point (belt surface) */
	FVector Location = FVector::ZeroVector;

	/** Flow direction at this end, zero if the belt may choose */
	FVector Direction = FVector::ZeroVector;

	/** True if items flow into this end (machine input or pole input side) */
	bool bBeltEndsHere = false;

	bool IsSameAs(const FFactoryBeltEnd& Other) const;
};

/**
 *  The build tool of the player: placing buildings, poles and belts, demolishing and emptying demolition crates.
 *  Lives on the AFactoryGamePlayerController, which forwards the build keys to it.
 *
 *  Ablauf Gebäude: Hotbar-Taste → Hologramm am Fadenkreuz (1-m-Raster, Mausrad dreht) → grün, wenn der Boden
 *  eben genug ist (max. 1 m Höhenunterschied, das Gebäude sinkt ein), nichts im Weg steht und das Material
 *  reicht → Linksklick baut und zieht die Kosten ab.
 *  Ablauf Band: 1. Klick auf einen Port, eine Stütze oder den Boden (dort entsteht eine neue Stütze),
 *  2. Klick auf das andere Ende. Beginnt man an einem Eingang, wird das Band rückwärts gezogen.
 *  Endet es nicht an einer Maschine, geht es von dort aus weiter (Kette), bis man rechtsklickt. R wechselt den Modus (Standard/Gerade/Kurve),
 *  das Mausrad die Höhe neuer Stützen.
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

	/** How long the interact key has to be held on a resource node for one ore, in seconds */
	static constexpr float HandMiningTime = 1.0f;

	/** How long the primary button has to be held to demolish, in seconds */
	static constexpr float DemolishHoldTime = 0.5f;

	/** How close the crosshair has to be to a port or pole to snap to it, in cm */
	static constexpr double SnapDistance = 150.0;

	/** Building table id of the conveyor pole */
	static const FName PoleId;

	UFactoryBuildComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	EFactoryBuildMode GetMode() const { return Mode; }

	/** Starts placing a building, pole or belt from the building table. Unknown ids stop building */
	void BeginPlacing(FName BuildingId);

	/** Switches the demolish mode on or off */
	void ToggleDemolish();

	/** Leaves the build mode */
	void StopBuilding();

	/**
	 *  Right mouse button / B: steps back. While drawing a belt this drops the chosen start first.
	 *  Returns true if the build mode was left
	 */
	bool Cancel();

	/** Primary button (left mouse, RT) pressed or released */
	void PrimaryPressed();
	void PrimaryReleased();

	/** Mouse wheel: rotates buildings, changes the pole height for poles and belts. Fine: Ctrl held */
	void Scroll(int32 Direction, bool bFine);

	/** R: cycles the belt mode */
	void CycleMode();

	/** Interact key (E): empties the demolition crate in view or opens the machine in view. Holding it on a resource node mines by hand */
	void Interact();

protected:

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	AFactoryGamePlayerController* GetPlayerController() const;

	const FFactoryBuildingDefinition* GetBuilding() const;

	/** Line trace along the view, ignoring the player and the hologram */
	bool TraceView(double Range, FHitResult& OutHit) const;

	/** Moves the hologram to the crosshair and checks if the machine can be built there */
	void UpdatePlacement();

	/** Same for a single pole */
	void UpdatePole();

	/** Finds the foundation cells under the crosshair (one, or the dragged area) and checks them */
	void UpdateFoundation();

	/** Where a single foundation goes for this hit: flush next to or on top of a foundation nearby, else free on the 1 m grid */
	FTransform ComputeFoundationCell(const FHitResult& Hit, const FFactoryBuildingDefinition& Building) const;

	/** Finds the belt end under the crosshair and checks the path */
	void UpdateBelt();

	/** Finds the demolish target and advances the hold timer */
	void UpdateDemolish(float DeltaTime);

	/** Shows the prompts for crates, machines and resource nodes, mines by hand while E is held on a node */
	void UpdateIdle(float DeltaTime);

	/** Builds the current placement. Returns false if it isn't valid */
	bool PlaceBuilding();
	bool PlacePole();
	bool PlaceBelt();
	bool PlaceFoundations();

	/**
	 *  Finds what a belt would connect to under the crosshair: a port or pole nearby wins over everything else,
	 *  otherwise a new pole on the ground. Returns false and sets OutError if nothing fits
	 */
	bool FindBeltEnd(EFactoryBeltEndWant Want, FFactoryBeltEnd& OutEnd, FText& OutError) const;

	/** Returns true if nothing (other buildings, belts, the player) is in a box. Used for poles */
	bool IsFree(const FVector& Center, const FQuat& Rotation, const FVector& HalfSize) const;

	/**
	 *  Returns true if the belt path doesn't run through a machine, crate, the ground or a rock.
	 *  Other belts and poles may be crossed, the first and last 75 cm are skipped because they touch the ends
	 */
	bool IsBeltPathFree(const FFactoryBeltPath& Path) const;

	/** Takes the costs from the inventory and shows "-N item" */
	void PayCosts(const TArray<FFactoryItemStack>& Costs) const;

	/** Spawns a pole for a belt end that needs a new one, facing along the belt */
	AFactoryBeltPole* SpawnPole(const FVector& Foot, int32 Height, const FVector& Direction) const;

	/** Demolishes a machine, belt or pole and gives everything back */
	void Demolish(AActor* Target);

	/** Highlights the demolish target (and removes the highlight from the old one) */
	void SetDemolishTarget(AActor* NewTarget);

	/** Returns the cost lines for the HUD and whether the player has everything */
	bool CheckCosts(const TArray<FFactoryItemStack>& Costs, TArray<TPair<FText, bool>>& OutLines) const;

	/** Sends the build panel to the HUD */
	void ShowPanel(const FText& Title, const TArray<TPair<FText, bool>>& Lines, const FText& Error, const FText& Controls) const;

	void HidePanel() const;

	/** Spawns the hologram if needed */
	AFactoryHologram* GetHologram();

	EFactoryBuildMode Mode = EFactoryBuildMode::None;

	/** Building, pole or belt being placed */
	FName BuildingId;

	/** Rotation of the hologram around the up axis */
	double PlaceYaw = 0.0;

	/** Current hologram transform and whether it can be built there */
	FTransform PlaceTransform;
	bool bPlaceValid = false;

	/** Height of new poles in meters, changed with the mouse wheel */
	int32 PoleHeight = 1;

	EFactoryBeltMode BeltMode = EFactoryBeltMode::Standard;

	/** Start of the belt being drawn, valid after the first click */
	bool bHasBeltStart = false;
	FFactoryBeltEnd BeltStart;

	/** End under the crosshair and the resulting path */
	FFactoryBeltEnd BeltEnd;
	FFactoryBeltPath BeltPath;

	UPROPERTY()
	TObjectPtr<AFactoryHologram> Hologram;

	UPROPERTY()
	TWeakObjectPtr<AActor> DemolishTarget;

	/** Foundation cells of the current placement (world transforms of the foundation origins) */
	TArray<FTransform> FoundationCells;

	/** True while the primary button is held in foundation mode; the area is spanned from FoundationAnchor */
	bool bFoundationDrag = false;
	FTransform FoundationAnchor;

	/** Resource node under a drill hologram, the drill gets its recipe from it */
	TWeakObjectPtr<AFactoryResourceNode> PlaceNode;

	/** Time the interact key has been held on a resource node */
	float MiningTime = 0.0f;

	/** True while the primary button is held in demolish mode */
	bool bHoldingPrimary = false;

	float HoldTime = 0.0f;
};
