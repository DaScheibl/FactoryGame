// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "FactoryMenuWidget.h"
#include "FactoryBuildMenuEntryWidget.generated.h"

class UBorder;
class UFactoryBuildMenuWidget;

/**
 *  A building dragged from the build menu onto a hotbar slot
 *  Wird beim Ziehen einer Kachel erzeugt; der Hotbar-Slot im Baumenü nimmt sie in NativeOnDrop an.
 *  Kommt sie von einem Hotbar-Slot, wird der Eintrag dorthin verschoben.
 */
UCLASS()
class FACTORYGAME_API UFactoryBuildingDragOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:

	FName BuildingId;

	/** Hotbar slot the building was dragged from, INDEX_NONE for a tile */
	int32 SourceHotbarSlot = INDEX_NONE;
};

/**
 *  One tile of the build menu, or one of the hotbar slots at the bottom of the build menu.
 *
 *  Zeigt nur an und meldet Eingaben ans Baumenü (UFactoryBuildMenuWidget): Klick, Ziehen,
 *  Ablegen, Hover und Fokus. Kacheln zeigen Name und Größe, Hotbar-Slots Tastennummer und Name.
 */
UCLASS()
class FACTORYGAME_API UFactoryBuildMenuEntryWidget : public UFactoryMenuWidget
{
	GENERATED_BODY()

public:

	static constexpr float TileWidth = 132.0f;
	static constexpr float TileHeight = 84.0f;
	static constexpr float HotbarSlotSize = 64.0f;

	/** Shows a building tile */
	void SetupTile(UFactoryBuildMenuWidget* InOwner, FName InBuildingId);

	/** Shows a hotbar slot */
	void SetupHotbarSlot(UFactoryBuildMenuWidget* InOwner, int32 InHotbarSlot);

	/** Shows a building that is not in the menu, e.g. while it's being dragged */
	void SetupPreview(FName InBuildingId);

	/** Building on the tile or hotbar slot, None for an empty slot */
	FName GetBuildingId() const;

	/** Hotbar slot index, INDEX_NONE for tiles */
	int32 GetHotbarSlot() const { return HotbarSlot; }

	bool IsHotbarSlot() const { return HotbarSlot != INDEX_NONE; }

protected:

	virtual UWidget* BuildContent() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:

	/** Updates the background for hover, focus and the held building */
	void UpdateBackground();

	TWeakObjectPtr<UFactoryBuildMenuWidget> Owner;

	FName BuildingId;

	int32 HotbarSlot = INDEX_NONE;

	/** Left mouse button went down on this entry and may become a click or a drag */
	bool bLeftPressed = false;

	bool bHovered = false;

	bool bFocused = false;

	UPROPERTY()
	TObjectPtr<UBorder> Background;
};
