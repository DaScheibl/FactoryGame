// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "FactoryMenuWidget.h"
#include "FactoryInventorySlotWidget.generated.h"

class UBorder;
class UFactoryInventoryComponent;
class UFactoryInventoryWidget;

/**
 *  A stack being dragged from an inventory slot
 *  Wird beim Ziehen mit der Maus erzeugt und beim Loslassen an den Ziel-Slot übergeben (NativeOnDrop).
 *  Die Items bleiben während des Ziehens im Quell-Slot; verschoben wird erst beim Ablegen.
 */
UCLASS()
class FACTORYGAME_API UFactoryItemDragOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:

	TWeakObjectPtr<UFactoryInventoryComponent> SourceInventory;

	int32 SourceIndex = INDEX_NONE;

	FName ItemId;

	int32 Count = 0;
};

/** Ways to click a slot. Mouse buttons and modifiers map to these, and so do the gamepad face buttons */
enum class EFactorySlotClick : uint8
{
	/** Left click / A: pick up the whole stack or put the held items down */
	Primary,

	/** Right click / X: pick up half a stack or put one held item down */
	Secondary,

	/** Shift + click / Y: move the stack to the other panel */
	QuickMove,

	/** Ctrl + click: move all stacks of this item to the other panel */
	MoveAll
};

/**
 *  One slot of the inventory window. Also used as the drag and cursor visual of a held stack.
 *
 *  Der Slot zeigt nur an und meldet Eingaben weiter: Maus und Gamepad werden in EFactorySlotClick
 *  übersetzt und an UFactoryInventoryWidget::HandleSlotClick gereicht. Die eigentliche Logik
 *  (aufnehmen, ablegen, verschieben) steckt im Inventarfenster.
 */
UCLASS()
class FACTORYGAME_API UFactoryInventorySlotWidget : public UFactoryMenuWidget
{
	GENERATED_BODY()

public:

	static constexpr float SlotSize = 64.0f;

	/** Shows a slot of an inventory */
	void Setup(UFactoryInventoryWidget* InOwner, UFactoryInventoryComponent* InInventory, int32 InIndex);

	/** Shows a stack that is not in a slot, e.g. while it's being dragged */
	void SetupPreview(FName InItemId, int32 InCount);

	UFactoryInventoryComponent* GetInventory() const { return Inventory.Get(); }

	int32 GetIndex() const { return Index; }

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

	/** Updates the background for hover, focus and held state */
	void UpdateBackground();

	/** Forwards a click to the inventory window */
	void Click(EFactorySlotClick ClickType);

	TWeakObjectPtr<UFactoryInventoryWidget> Owner;

	TWeakObjectPtr<UFactoryInventoryComponent> Inventory;

	int32 Index = INDEX_NONE;

	/** Stack shown in preview mode */
	FName PreviewItemId;
	int32 PreviewCount = 0;

	/** Left mouse button went down on this slot and may become a click or a drag */
	bool bLeftPressed = false;

	bool bHovered = false;

	bool bFocused = false;

	UPROPERTY()
	TObjectPtr<UBorder> Background;
};
