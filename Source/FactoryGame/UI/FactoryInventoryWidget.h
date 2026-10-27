// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FactoryMenuWidget.h"
#include "FactoryInventorySlotWidget.h"
#include "FactoryInventoryWidget.generated.h"

class UCanvasPanelSlot;
class UFactoryInventoryComponent;

/**
 *  Inventory window with the equipment slots on the left and the inventory grid on the right.
 *  Stacks can be picked up onto the cursor, dragged, split with the right mouse button and quick moved with shift/ctrl,
 *  and the same actions are on the gamepad face buttons.
 *  A held stack stays in its slot until it is put down, so closing the window never loses items.
 *
 *  Aufbau: links die Ausrüstungs-Slots, rechts das Raster mit 10 Spalten (scrollt ab 6 Reihen),
 *  darunter Detailzeile, Steuerungshinweis und Buttons. Jeder Slot ist ein
 *  UFactoryInventorySlotWidget, das seine Klicks hierher zurückmeldet.
 *  "Held" ist der Stapel am Mauszeiger: gemerkt werden nur Inventar, Slot und Anzahl, die Items
 *  selbst bleiben im Slot. Erst beim Ablegen verschiebt MoveTo() sie wirklich.
 *  Bei jeder Inventar-Änderung wird das Fenster neu gebaut (HandleInventoryChanged).
 */
UCLASS()
class FACTORYGAME_API UFactoryInventoryWidget : public UFactoryMenuWidget
{
	GENERATED_BODY()

public:

	/** Called when the player wants to close the inventory */
	FSimpleDelegate OnCloseRequested;

	/** Sets the inventory and equipment to show */
	void SetInventory(UFactoryInventoryComponent* InInventory, UFactoryInventoryComponent* InEquipment);

	/** Gives the keyboard/gamepad focus to the last used slot */
	void FocusCurrentSlot();

	/** Handles a click on a slot */
	void HandleSlotClick(UFactoryInventorySlotWidget* SlotWidget, EFactorySlotClick ClickType);

	/** Handles a stack dropped onto a slot */
	void HandleSlotDrop(UFactoryInventorySlotWidget* SlotWidget, const UFactoryItemDragOperation* Operation);

	/** Shows the item of a slot in the details area, or the held stack / the controls hint if nullptr */
	void ShowDetails(UFactoryInventorySlotWidget* SlotWidget);

	/** Remembers the focused slot and shows its details when using a gamepad */
	void HandleSlotFocused(UFactoryInventorySlotWidget* SlotWidget);

	/** Returns how many items of the slot are held on the cursor */
	int32 GetHeldCount(const UFactoryInventoryComponent* SlotInventory, int32 Index) const;

	bool IsHolding() const { return Held.Count > 0; }

	bool IsUsingGamepad() const { return bUsingGamepad; }

	/** Switches the hints and focus highlight between mouse and gamepad */
	void SetUsingGamepad(bool bInUsingGamepad);

protected:

	/** Starts listening to inventory changes */
	virtual void NativeConstruct() override;

	/** Stops listening to inventory changes and puts the held stack back */
	virtual void NativeDestruct() override;

	/** Closes the inventory with the inventory keys */
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	/** Puts the held stack back when clicking next to the slots */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Moves the held stack with the cursor */
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Builds the window layout */
	virtual UWidget* BuildContent() override;

	/** Puts the held stack back, or closes the inventory */
	virtual bool HandleEscape() override;

private:

	/** Items picked up from a slot. They stay in the slot until they are put down */
	struct FHeldStack
	{
		TWeakObjectPtr<UFactoryInventoryComponent> Inventory;
		int32 Index = INDEX_NONE;
		FName ItemId;
		int32 Count = 0;
	};

	UWidget* BuildSlot(UFactoryInventoryComponent* SlotInventory, int32 Index);
	UWidget* BuildDetails();

	/** Returns the panel a quick move goes to */
	UFactoryInventoryComponent* GetOtherInventory(const UFactoryInventoryComponent* SlotInventory) const;

	/** Puts the held stack back into its slot */
	void ReleaseHeld();

	/** Drops the held stack if its slot changed */
	void ValidateHeld();

	/** Rebuilds and keeps the focus on the last used slot */
	void Refresh();

	/** Rebuilds when the inventory changed */
	void HandleInventoryChanged();

	/** Shows the controls for mouse or gamepad */
	void UpdateHint();

	/** Places the held stack preview next to the cursor */
	void UpdateHeldPreviewPosition();

	UFactoryInventorySlotWidget* FindSlotWidget(const UFactoryInventoryComponent* SlotInventory, int32 Index) const;

	TWeakObjectPtr<UFactoryInventoryComponent> Inventory;
	TWeakObjectPtr<UFactoryInventoryComponent> Equipment;

	FDelegateHandle InventoryChangedHandle;
	FDelegateHandle EquipmentChangedHandle;

	FHeldStack Held;

	/** True while a click or drop moves items. The change events of that move are ignored, the click handler validates and refreshes itself afterwards */
	bool bApplyingMove = false;

	/** Slot that gets the focus after a rebuild */
	TWeakObjectPtr<UFactoryInventoryComponent> FocusInventory;
	int32 FocusIndex = 0;

	bool bUsingGamepad = false;

	/** Last cursor position in widget space */
	FVector2D CursorPosition = FVector2D::ZeroVector;

	/** Slot widgets of the current layout */
	UPROPERTY()
	TArray<TObjectPtr<UFactoryInventorySlotWidget>> SlotWidgets;

	UPROPERTY()
	TObjectPtr<UTextBlock> DetailsTitle;

	UPROPERTY()
	TObjectPtr<UTextBlock> DetailsInfo;

	UPROPERTY()
	TObjectPtr<UTextBlock> DetailsDescription;

	UPROPERTY()
	TObjectPtr<UTextBlock> HintText;

	/** Slot of the held stack preview on the cursor layer */
	UPROPERTY()
	TObjectPtr<UCanvasPanelSlot> HeldPreviewSlot;
};
