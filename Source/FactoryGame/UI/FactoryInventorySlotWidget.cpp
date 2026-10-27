// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryInventorySlotWidget.h"
#include "FactoryInventoryWidget.h"
#include "FactoryInventoryComponent.h"
#include "FactoryItemDatabase.h"
#include "FactoryLocalization.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Brushes/SlateRoundedBoxBrush.h"

namespace
{
	const float SlotCornerRadius = 6.0f;
	const FLinearColor SlotColor = FLinearColor(FColor(28, 30, 36, 220));
	const FLinearColor SlotHighlightColor = FLinearColor(FColor(62, 50, 40, 235));

	FText GetEquipSlotName(EFactoryEquipSlot EquipSlot)
	{
		switch (EquipSlot)
		{
		case EFactoryEquipSlot::Head:	return FactoryLoc::Get("Equip_Head");
		case EFactoryEquipSlot::Body:	return FactoryLoc::Get("Equip_Body");
		case EFactoryEquipSlot::Back:	return FactoryLoc::Get("Equip_Back");
		case EFactoryEquipSlot::Hand:	return FactoryLoc::Get("Equip_Hand");
		default:						return FText::GetEmpty();
		}
	}
}

void UFactoryInventorySlotWidget::Setup(UFactoryInventoryWidget* InOwner, UFactoryInventoryComponent* InInventory, int32 InIndex)
{
	Owner = InOwner;
	Inventory = InInventory;
	Index = InIndex;
	RebuildMenu();
}

void UFactoryInventorySlotWidget::SetupPreview(FName InItemId, int32 InCount)
{
	PreviewItemId = InItemId;
	PreviewCount = InCount;
	SetVisibility(ESlateVisibility::HitTestInvisible);
	RebuildMenu();
}

UWidget* UFactoryInventorySlotWidget::BuildContent()
{
	FName ItemId = PreviewItemId;
	int32 Count = PreviewCount;
	bool bHeldHere = false;

	if (Inventory.IsValid() && Inventory->GetSlots().IsValidIndex(Index))
	{
		const FFactoryItemStack& Stack = Inventory->GetSlots()[Index];
		const int32 HeldCount = Owner.IsValid() ? Owner->GetHeldCount(Inventory.Get(), Index) : 0;

		ItemId = Stack.ItemId;
		Count = Stack.IsEmpty() ? 0 : Stack.Count - HeldCount;
		bHeldHere = HeldCount > 0;
	}

	const UFactoryItemDatabase* Database = UFactoryItemDatabase::Get(this);
	const FFactoryItemDefinition* Item = Database && !ItemId.IsNone() ? Database->FindItem(ItemId) : nullptr;

	UOverlay* SlotContent = NewWidget<UOverlay>();

	if (!ItemId.IsNone() && (Count > 0 || bHeldHere))
	{
		// placeholder until the icon rig renders real icons. A fully picked up stack stays as a faint ghost
		const FLinearColor ItemColor = Item ? FLinearColor(Item->PlaceholderColor) : ErrorColor;

		UBorder* Swatch = NewWidget<UBorder>();
		Swatch->SetBrush(FSlateRoundedBoxBrush(ItemColor.CopyWithNewOpacity(Count > 0 ? 0.55f : 0.15f), 4.0f));

		if (UOverlaySlot* SwatchSlot = SlotContent->AddChildToOverlay(Swatch))
		{
			SwatchSlot->SetHorizontalAlignment(HAlign_Fill);
			SwatchSlot->SetVerticalAlignment(VAlign_Fill);
		}

		if (Count > 0)
		{
			UTextBlock* NameText = MakeText(Item ? Item->GetDisplayName() : FText::FromName(ItemId), 9.0f);
			NameText->SetJustification(ETextJustify::Center);
			NameText->SetAutoWrapText(true);

			if (UOverlaySlot* NameSlot = SlotContent->AddChildToOverlay(NameText))
			{
				NameSlot->SetHorizontalAlignment(HAlign_Fill);
				NameSlot->SetVerticalAlignment(VAlign_Top);
				NameSlot->SetPadding(FMargin(3.0f, 4.0f, 3.0f, 0.0f));
			}

			// single items like tools don't need a count
			if (Count > 1 || (Item && Item->GetMaxStackSize() > 1))
			{
				UTextBlock* CountText = MakeText(FText::AsNumber(Count), 13.0f, TextColor, true);

				if (UOverlaySlot* CountSlot = SlotContent->AddChildToOverlay(CountText))
				{
					CountSlot->SetHorizontalAlignment(HAlign_Right);
					CountSlot->SetVerticalAlignment(VAlign_Bottom);
					CountSlot->SetPadding(FMargin(0.0f, 0.0f, 5.0f, 2.0f));
				}
			}
		}
	}
	else if (Inventory.IsValid() && Inventory->GetSlotFilter(Index) != EFactoryEquipSlot::None)
	{
		// empty equipment slots show what belongs there
		UTextBlock* SlotName = MakeText(GetEquipSlotName(Inventory->GetSlotFilter(Index)), 11.0f, SubtleTextColor);
		SlotName->SetJustification(ETextJustify::Center);

		if (UOverlaySlot* NameSlot = SlotContent->AddChildToOverlay(SlotName))
		{
			NameSlot->SetHorizontalAlignment(HAlign_Center);
			NameSlot->SetVerticalAlignment(VAlign_Center);
		}
	}

	if (Item && Inventory.IsValid())
	{
		SetToolTipText(FText::Format(FText::FromString(TEXT("{0}\n{1}")), Item->GetDisplayName(), Item->GetDescription()));
	}
	else
	{
		SetToolTipText(FText::GetEmpty());
	}

	Background = NewWidget<UBorder>();
	Background->SetPadding(FMargin(2.0f));
	Background->SetContent(SlotContent);
	UpdateBackground();

	return MakeSized(Background, SlotSize, SlotSize);
}

void UFactoryInventorySlotWidget::UpdateBackground()
{
	if (!Background)
	{
		return;
	}

	const bool bHeldHere = Owner.IsValid() && Inventory.IsValid() && Owner->GetHeldCount(Inventory.Get(), Index) > 0;
	const bool bShowFocus = bFocused && Owner.IsValid() && Owner->IsUsingGamepad();

	if (bHeldHere)
	{
		Background->SetBrush(FSlateRoundedBoxBrush(SlotHighlightColor, SlotCornerRadius, AccentColor, 1.5f));
	}
	else if (bShowFocus)
	{
		Background->SetBrush(FSlateRoundedBoxBrush(SlotHighlightColor, SlotCornerRadius, AccentColor, 2.0f));
	}
	else if (bHovered)
	{
		Background->SetBrush(FSlateRoundedBoxBrush(SlotHighlightColor, SlotCornerRadius, AccentColor, 1.0f));
	}
	else
	{
		Background->SetBrush(FSlateRoundedBoxBrush(SlotColor, SlotCornerRadius));
	}
}

void UFactoryInventorySlotWidget::Click(EFactorySlotClick ClickType)
{
	if (Owner.IsValid())
	{
		Owner->HandleSlotClick(this, ClickType);
	}
}

FReply UFactoryInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!Owner.IsValid() || !Inventory.IsValid())
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	Owner->SetUsingGamepad(false);

	const FKey Button = InMouseEvent.GetEffectingButton();

	if (Button == EKeys::RightMouseButton)
	{
		Click(EFactorySlotClick::Secondary);
		return FReply::Handled();
	}

	if (Button != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	if (InMouseEvent.IsShiftDown())
	{
		Click(EFactorySlotClick::QuickMove);
		return FReply::Handled();
	}

	if (InMouseEvent.IsControlDown())
	{
		Click(EFactorySlotClick::MoveAll);
		return FReply::Handled();
	}

	// while holding a stack the press puts it down, otherwise wait whether this becomes a click or a drag
	if (Owner->IsHolding())
	{
		Click(EFactorySlotClick::Primary);
		return FReply::Handled();
	}

	bLeftPressed = true;
	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

FReply UFactoryInventorySlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bLeftPressed)
	{
		bLeftPressed = false;
		Click(EFactorySlotClick::Primary);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UFactoryInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	bLeftPressed = false;

	if (!Owner.IsValid() || !Inventory.IsValid() || !Inventory->GetSlots().IsValidIndex(Index) || Inventory->GetSlots()[Index].IsEmpty())
	{
		return;
	}

	const FFactoryItemStack& Stack = Inventory->GetSlots()[Index];

	UFactoryItemDragOperation* Operation = NewObject<UFactoryItemDragOperation>();
	Operation->SourceInventory = Inventory;
	Operation->SourceIndex = Index;
	Operation->ItemId = Stack.ItemId;
	Operation->Count = Stack.Count;

	UFactoryInventorySlotWidget* Visual = CreateWidget<UFactoryInventorySlotWidget>(Owner.Get(), UFactoryInventorySlotWidget::StaticClass());
	Visual->SetupPreview(Stack.ItemId, Stack.Count);

	Operation->DefaultDragVisual = Visual;
	Operation->Pivot = EDragPivot::CenterCenter;

	OutOperation = Operation;
}

bool UFactoryInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UFactoryItemDragOperation* Operation = Cast<UFactoryItemDragOperation>(InOperation);

	if (!Operation || !Owner.IsValid())
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	Owner->HandleSlotDrop(this, Operation);
	return true;
}

void UFactoryInventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	bHovered = true;
	UpdateBackground();

	if (Owner.IsValid())
	{
		Owner->ShowDetails(this);
	}
}

void UFactoryInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	bHovered = false;
	bLeftPressed = false;
	UpdateBackground();

	if (Owner.IsValid())
	{
		Owner->ShowDetails(nullptr);
	}
}

void UFactoryInventorySlotWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	bFocused = true;
	UpdateBackground();

	if (Owner.IsValid())
	{
		Owner->HandleSlotFocused(this);
	}
}

void UFactoryInventorySlotWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	bFocused = false;
	UpdateBackground();
}

FReply UFactoryInventorySlotWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Owner.IsValid() && Key.IsGamepadKey() && !Owner->IsUsingGamepad())
	{
		Owner->SetUsingGamepad(true);
		UpdateBackground();
	}

	if (Key == EKeys::Gamepad_FaceButton_Bottom || Key == EKeys::Enter || Key == EKeys::SpaceBar)
	{
		Click(EFactorySlotClick::Primary);
		return FReply::Handled();
	}

	if (Key == EKeys::Gamepad_FaceButton_Left)
	{
		Click(EFactorySlotClick::Secondary);
		return FReply::Handled();
	}

	if (Key == EKeys::Gamepad_FaceButton_Top)
	{
		Click(EFactorySlotClick::QuickMove);
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
