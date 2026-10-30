// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FactoryGamePlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UPauseMenuWidget;
class UFactoryHUDWidget;
class UFactoryInventoryWidget;
class UFactoryBuildMenuWidget;
class UFactoryMachineWidget;
class UFactoryHubWidget;
class AFactoryHub;
class AFactoryMachine;
struct FFactoryItemStack;
class UFactoryBuildComponent;

/**
 *  Simple first person Player Controller
 *  Manages the input mapping context.
 *  Overrides the Player Camera Manager class.
 *
 *  Zentrale Stelle für alles, was der Spieler im Spiel bedient:
 *  - erzeugt HUD, Pausemenü, Inventar und Baumenü (C++-Widgets, kein Widget Blueprint)
 *  - bindet die festen Tasten: Esc/P = Pause, Tab/I = Inventar, Q = Baumenü, 1-0 = Hotbar,
 *    Bauen: Linksklick, Mausrad, R (Band-Modus), Rechtsklick, F (Abriss), E (Kiste leeren), siehe Docs/Standards/13-bauen.md
 *  - besitzt das Bauwerkzeug (UFactoryBuildComponent); ein Gebäude auf der Hotbar wählen startet das Platzieren
 *  - verwaltet die Hotbar (10 Slots mit Bau-Shortcuts), die mit der Welt gespeichert wird
 *  - schaltet zwischen Spiel-Eingabe (Maus dreht Kamera) und UI-Eingabe (Mauszeiger) um
 *  - registriert den Cheat Manager für Konsolenbefehle zum Testen
 */
UCLASS(abstract, config="Game")
class FACTORYGAME_API AFactoryGamePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	AFactoryGamePlayerController();

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Pause menu widget to spawn */
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

	/** Pointer to the pause menu widget */
	UPROPERTY()
	TObjectPtr<UPauseMenuWidget> PauseMenuWidget;

	/** HUD widget to spawn */
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UFactoryHUDWidget> HUDWidgetClass;

	/** Pointer to the HUD widget */
	UPROPERTY()
	TObjectPtr<UFactoryHUDWidget> HUDWidget;

	/** Inventory widget to spawn */
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UFactoryInventoryWidget> InventoryWidgetClass;

	/** Pointer to the inventory widget */
	UPROPERTY()
	TObjectPtr<UFactoryInventoryWidget> InventoryWidget;

	/** Build menu widget to spawn */
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UFactoryBuildMenuWidget> BuildMenuWidgetClass;

	/** Pointer to the build menu widget */
	UPROPERTY()
	TObjectPtr<UFactoryBuildMenuWidget> BuildMenuWidget;

	/** Machine panel widget to spawn */
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UFactoryMachineWidget> MachineWidgetClass;

	/** Pointer to the machine panel widget */
	UPROPERTY()
	TObjectPtr<UFactoryMachineWidget> MachineWidget;

	/** HUB window widget to spawn */
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UFactoryHubWidget> HubWidgetClass;

	/** Pointer to the HUB window widget */
	UPROPERTY()
	TObjectPtr<UFactoryHubWidget> HubWidget;

	/** HUB whose storage is open in the inventory window */
	TWeakObjectPtr<AFactoryHub> StorageHub;

	/** Build tool: placing, demolishing, emptying crates */
	UPROPERTY(VisibleAnywhere, Category="Build")
	TObjectPtr<UFactoryBuildComponent> BuildComponent;

	/** Build shortcuts on the hotbar, None for an empty slot */
	UPROPERTY()
	TArray<FName> HotbarSlots;

	/** Selected hotbar slot, INDEX_NONE if nothing is selected */
	int32 SelectedHotbarSlot = INDEX_NONE;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Saves the world when the game is closed */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Restores the saved player position after a world has been loaded */
	virtual void OnPossess(APawn* InPawn) override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	/** Selects the hotbar slot of a number key */
	void HandleHotbarKey(FKey Key);

	/** Selects the previous or next hotbar slot */
	void SelectPreviousHotbarSlot();
	void SelectNextHotbarSlot();

	/** Switches back to game input if no menu is open */
	void RestoreGameInput();

	/** Closes inventory, build menu and machine panel; only one of them is open at a time */
	void CloseWindows();

	/** Build keys, forwarded to the build tool */
	void HandlePrimaryPressed();
	void HandlePrimaryReleased();
	void HandleCancel();
	void HandleRotateLeft();
	void HandleRotateRight();
	void HandleToggleDemolish();
	void HandleCycleMode();
	void HandleInteract();

	/** Escape/P: leaves the build mode first, opens the pause menu otherwise */
	void HandlePauseKey();

	/** Puts the first buildings of the table on an empty hotbar (new worlds) */
	void FillDefaultHotbar();

public:

	/** Opens or closes the pause menu */
	void TogglePauseMenu();

	/** Pauses the game and shows the pause menu */
	void OpenPauseMenu();

	/** Hides the pause menu and resumes the game */
	void ClosePauseMenu();

	/** Opens or closes the inventory */
	void ToggleInventory();

	/** Shows the inventory of the controlled pawn */
	void OpenInventory();

	/** Hides the inventory */
	void CloseInventory();

	/** Opens or closes the build menu */
	void ToggleBuildMenu();

	/** Shows the build menu, replacing the inventory */
	void OpenBuildMenu();

	/** Hides the build menu */
	void CloseBuildMenu();

	/** Closes the build menu and starts placing a building. Selects its hotbar slot if it has one */
	void PickBuilding(FName BuildingId);

	/** Shows the panel of a machine (E), replacing the other windows */
	void OpenMachinePanel(AFactoryMachine* Machine);

	/** Hides the machine panel */
	void CloseMachinePanel();

	/** Shows the HUB window (E on the HUB) */
	void OpenHubPanel(AFactoryHub* Hub);

	/** Hides the HUB window */
	void CloseHubPanel();

	/** Shows the inventory with the HUB storage above it */
	void OpenHubStorage();

	/** Gives items to the pawn; what doesn't fit goes into a demolition crate at the location */
	void GiveItemsOrCrate(const TArray<FFactoryItemStack>& Stacks, const FVector& CrateLocation);

	/** Adds items to the pawn's inventory and shows a notification. Returns the number of items that fit */
	int32 GiveItems(FName ItemId, int32 Count);

	/** Returns the HUD, may be nullptr */
	UFactoryHUDWidget* GetHUDWidget() const { return HUDWidget; }

	/** Anzahl der Hotbar-Slots: Tasten 1-9 und 0 */
	static constexpr int32 NumHotbarSlots = 10;

	/** Returns the shortcut on the given hotbar slot */
	FName GetHotbarSlot(int32 Index) const { return HotbarSlots.IsValidIndex(Index) ? HotbarSlots[Index] : NAME_None; }

	/** Puts a shortcut on a hotbar slot */
	void SetHotbarSlot(int32 Index, FName Shortcut);

	/** Returns all hotbar shortcuts, used for saving */
	const TArray<FName>& GetHotbarSlots() const { return HotbarSlots; }

	/** Replaces all hotbar shortcuts, used for loading */
	void SetHotbarSlots(const TArray<FName>& InSlots);

	/** Gewählter Hotbar-Slot, INDEX_NONE wenn keiner gewählt ist */
	int32 GetSelectedHotbarSlot() const { return SelectedHotbarSlot; }

	/** Selects a hotbar slot, or deselects it if it's already selected */
	void SelectHotbarSlot(int32 Index);

	UFactoryBuildComponent* GetBuildComponent() const { return BuildComponent; }
};
