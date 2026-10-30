// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FactoryMenuWidget.h"
#include "FactoryBuildMenuWidget.generated.h"

class UEditableTextBox;
class UVerticalBox;
class UFactoryBuildMenuEntryWidget;
class UFactoryBuildingDragOperation;
class AFactoryGamePlayerController;

/**
 *  Build menu (Q): category tabs on the left, building tiles on the right, a search box on top,
 *  the details of the hovered building and the hotbar at the bottom.
 *  Clicking a tile closes the menu and starts placing the building. Tiles can be dragged onto the hotbar,
 *  or put there with the number keys while hovering them. The game keeps running while the menu is open.
 *
 *  Aufbau wie das Inventar: der Zustand (Kategorie, Suchtext, gehaltenes Gebäude) liegt in Membern,
 *  BuildContent() baut daraus das Fenster. Beim Tippen in die Suche wird nur das Kachel-Raster
 *  neu gefüllt (RefreshTiles), damit das Suchfeld den Fokus behält.
 *  Gamepad: LB/RB = Kategorie, A = bauen, Y = Gebäude aufnehmen und mit A auf einen Hotbar-Slot legen,
 *  X auf einem Hotbar-Slot = leeren, B = zurück/schließen. Regeln: Docs/Standards/13-bauen.md
 */
UCLASS()
class FACTORYGAME_API UFactoryBuildMenuWidget : public UFactoryMenuWidget
{
	GENERATED_BODY()

public:

	/** Called when the player wants to close the menu */
	FSimpleDelegate OnCloseRequested;

	/** Gives the keyboard/gamepad focus to the last used tile */
	void FocusCurrentEntry();

	/** Left click / A on a tile or hotbar slot */
	void HandleEntryClick(UFactoryBuildMenuEntryWidget* Entry);

	/** Right click / X on a hotbar slot clears it */
	void HandleEntryClear(UFactoryBuildMenuEntryWidget* Entry);

	/** Gamepad Y on a tile: picks the building up to put it on a hotbar slot */
	void HandleEntryHold(UFactoryBuildMenuEntryWidget* Entry);

	/** A building dragged onto a hotbar slot */
	void HandleEntryDrop(UFactoryBuildMenuEntryWidget* Entry, const UFactoryBuildingDragOperation* Operation);

	/** Number key on a hovered tile: puts the building on that hotbar slot */
	bool HandleNumberKey(const FKey& Key);

	/** Shows a building in the details area, or the hovered/focused one if None */
	void ShowDetails(FName BuildingId);

	/** Remembers the focused entry and shows its details */
	void HandleEntryFocused(UFactoryBuildMenuEntryWidget* Entry);

	/** Mouse hover changed, the number keys go to the hovered tile */
	void HandleEntryHovered(UFactoryBuildMenuEntryWidget* Entry, bool bHovered);

	/** Building picked up with gamepad Y, None if nothing is held */
	FName GetHeldBuilding() const { return HeldBuilding; }

	bool IsUsingGamepad() const { return bUsingGamepad; }

	/** Switches the hints and focus highlight between mouse and gamepad */
	void SetUsingGamepad(bool bInUsingGamepad);

	AFactoryGamePlayerController* GetFactoryController() const;

protected:

	/** Starts with the first category and an empty search */
	virtual void NativeConstruct() override;

	/** Closes with Q, switches categories with LB/RB, number keys put the hovered tile on the hotbar */
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	/** Switches the hints back to the mouse */
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Drops a held building first, closes the menu otherwise */
	virtual bool HandleEscape() override;

	virtual UWidget* BuildContent() override;

private:

	UWidget* BuildCategories();
	UWidget* BuildHotbar();

	/** Refills the tile grid for the current category or search */
	void RefreshTiles();

	/** Rebuilds the window and keeps the focus */
	void Refresh();

	/** Categories that have at least one building, in menu order */
	TArray<FName> GetVisibleCategories() const;

	/** Buildings shown in the grid: matches of the search in all categories, otherwise the current category */
	TArray<FName> GetShownBuildings() const;

	/** Moves to the previous or next category */
	void StepCategory(int32 Direction);

	/** Puts a building on a hotbar slot. A building that already was on another slot is moved */
	void AssignHotbarSlot(int32 Index, FName BuildingId);

	UFUNCTION()
	void HandleSearchChanged(const FText& Text);

	UFUNCTION()
	void HandleSearchCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFactoryBuildMenuEntryWidget* FindEntry(FName BuildingId, int32 HotbarSlot) const;

	void UpdateHint();

	FName CurrentCategory;

	FString SearchText;

	/** Building picked up with gamepad Y */
	FName HeldBuilding;

	/** Entry that gets the focus after a rebuild: a tile (FocusBuilding) or a hotbar slot */
	FName FocusBuilding;
	int32 FocusHotbarSlot = INDEX_NONE;

	/** Tile under the mouse, gets the number keys */
	FName HoveredBuilding;

	bool bUsingGamepad = false;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> SearchBox;

	UPROPERTY()
	TObjectPtr<UVerticalBox> TileGrid;

	UPROPERTY()
	TObjectPtr<UVerticalBox> DetailsBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> HintText;

	UPROPERTY()
	TArray<TObjectPtr<UFactoryBuildMenuEntryWidget>> TileWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UFactoryBuildMenuEntryWidget>> HotbarWidgets;
};
