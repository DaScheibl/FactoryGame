// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryInventoryWidget.h"
#include "FactoryInventoryComponent.h"
#include "FactoryItemDatabase.h"
#include "FactoryLocalization.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

namespace
{
	const int32 NumColumns = 10;
	const float SlotSpacing = 6.0f;
	const float SlotPitch = UFactoryInventorySlotWidget::SlotSize + SlotSpacing;

	/** Grid height before the slots start scrolling, enough for the fully upgraded inventory */
	const float MaxGridHeight = 6 * SlotPitch;

	/** Gap between the equipment column and the grid */
	const float ColumnGap = 28.0f;

	/** Offset of the held stack from the cursor tip */
	const FVector2D HeldPreviewOffset = FVector2D(10.0f, 10.0f);
}

void UFactoryInventoryWidget::SetInventory(UFactoryInventoryComponent* InInventory, UFactoryInventoryComponent* InEquipment)
{
	if (Inventory.Get() == InInventory && Equipment.Get() == InEquipment)
	{
		return;
	}

	if (Inventory.IsValid())
	{
		Inventory->OnInventoryChanged.Remove(InventoryChangedHandle);
	}

	if (Equipment.IsValid())
	{
		Equipment->OnInventoryChanged.Remove(EquipmentChangedHandle);
	}

	Inventory = InInventory;
	Equipment = InEquipment;
	Held = FHeldStack();
	FocusInventory = Inventory;
	FocusIndex = 0;

	if (IsInViewport())
	{
		if (Inventory.IsValid())
		{
			InventoryChangedHandle = Inventory->OnInventoryChanged.AddUObject(this, &UFactoryInventoryWidget::HandleInventoryChanged);
		}

		if (Equipment.IsValid())
		{
			EquipmentChangedHandle = Equipment->OnInventoryChanged.AddUObject(this, &UFactoryInventoryWidget::HandleInventoryChanged);
		}
	}

	Refresh();
}

void UFactoryInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Inventory.IsValid())
	{
		Inventory->OnInventoryChanged.Remove(InventoryChangedHandle);
		InventoryChangedHandle = Inventory->OnInventoryChanged.AddUObject(this, &UFactoryInventoryWidget::HandleInventoryChanged);
	}

	if (Equipment.IsValid())
	{
		Equipment->OnInventoryChanged.Remove(EquipmentChangedHandle);
		EquipmentChangedHandle = Equipment->OnInventoryChanged.AddUObject(this, &UFactoryInventoryWidget::HandleInventoryChanged);
	}

	Held = FHeldStack();
	FocusInventory = Inventory;
	FocusIndex = 0;
	Refresh();
}

void UFactoryInventoryWidget::NativeDestruct()
{
	if (Inventory.IsValid())
	{
		Inventory->OnInventoryChanged.Remove(InventoryChangedHandle);
	}

	if (Equipment.IsValid())
	{
		Equipment->OnInventoryChanged.Remove(EquipmentChangedHandle);
	}

	// the held items never left their slot
	Held = FHeldStack();

	Super::NativeDestruct();
}

FReply UFactoryInventoryWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key.IsGamepadKey())
	{
		SetUsingGamepad(true);
	}

	if (Key == EKeys::Tab || Key == EKeys::I || Key == EKeys::Gamepad_Special_Left)
	{
		OnCloseRequested.ExecuteIfBound();
		return FReply::Handled();
	}

	// gamepad B puts a held stack back first, then closes
	if (Key == EKeys::Gamepad_FaceButton_Right)
	{
		HandleEscape();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UFactoryInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (IsHolding())
	{
		ReleaseHeld();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UFactoryInventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	CursorPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

	// ignore the tiny moves some gamepads/drivers report so the hints don't flicker
	if (bUsingGamepad && !InMouseEvent.GetCursorDelta().IsNearlyZero(2.0f))
	{
		SetUsingGamepad(false);
	}

	UpdateHeldPreviewPosition();

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

bool UFactoryInventoryWidget::HandleEscape()
{
	if (IsHolding())
	{
		ReleaseHeld();
	}
	else
	{
		OnCloseRequested.ExecuteIfBound();
	}

	return true;
}

void UFactoryInventoryWidget::SetUsingGamepad(bool bInUsingGamepad)
{
	if (bUsingGamepad == bInUsingGamepad)
	{
		return;
	}

	bUsingGamepad = bInUsingGamepad;

	// the cursor preview and focus highlight depend on the input device
	Refresh();
}

void UFactoryInventoryWidget::HandleInventoryChanged()
{
	ValidateHeld();
	Refresh();
}

void UFactoryInventoryWidget::ValidateHeld()
{
	if (!IsHolding())
	{
		return;
	}

	const UFactoryInventoryComponent* HeldInventory = Held.Inventory.Get();

	if (!HeldInventory || !HeldInventory->GetSlots().IsValidIndex(Held.Index) || HeldInventory->GetSlots()[Held.Index].ItemId != Held.ItemId)
	{
		Held = FHeldStack();
		return;
	}

	Held.Count = FMath::Min(Held.Count, HeldInventory->GetSlots()[Held.Index].Count);

	if (Held.Count <= 0)
	{
		Held = FHeldStack();
	}
}

void UFactoryInventoryWidget::ReleaseHeld()
{
	Held = FHeldStack();
	Refresh();
}

int32 UFactoryInventoryWidget::GetHeldCount(const UFactoryInventoryComponent* SlotInventory, int32 Index) const
{
	return IsHolding() && Held.Inventory.Get() == SlotInventory && Held.Index == Index ? Held.Count : 0;
}

UFactoryInventoryComponent* UFactoryInventoryWidget::GetOtherInventory(const UFactoryInventoryComponent* SlotInventory) const
{
	// TODO: chests and machines become the other panel once they exist
	return SlotInventory == Equipment.Get() ? Inventory.Get() : Equipment.Get();
}

void UFactoryInventoryWidget::Refresh()
{
	RebuildMenu();
	FocusCurrentSlot();
	ShowDetails(FindSlotWidget(FocusInventory.Get(), FocusIndex));
}

UFactoryInventorySlotWidget* UFactoryInventoryWidget::FindSlotWidget(const UFactoryInventoryComponent* SlotInventory, int32 Index) const
{
	for (UFactoryInventorySlotWidget* SlotWidget : SlotWidgets)
	{
		if (SlotWidget->GetInventory() == SlotInventory && SlotWidget->GetIndex() == Index)
		{
			return SlotWidget;
		}
	}

	return nullptr;
}

void UFactoryInventoryWidget::FocusCurrentSlot()
{
	if (!IsInViewport())
	{
		return;
	}

	// the focus has to stay inside the window, otherwise the close keys stop working
	UFactoryInventorySlotWidget* SlotWidget = FindSlotWidget(FocusInventory.Get(), FocusIndex);

	if (!SlotWidget && SlotWidgets.Num() > 0)
	{
		SlotWidget = SlotWidgets[0];
	}

	if (SlotWidget)
	{
		SlotWidget->SetKeyboardFocus();
	}
}

void UFactoryInventoryWidget::HandleSlotFocused(UFactoryInventorySlotWidget* SlotWidget)
{
	FocusInventory = SlotWidget->GetInventory();
	FocusIndex = SlotWidget->GetIndex();

	if (bUsingGamepad)
	{
		ShowDetails(SlotWidget);
	}
}

void UFactoryInventoryWidget::HandleSlotClick(UFactoryInventorySlotWidget* SlotWidget, EFactorySlotClick ClickType)
{
	UFactoryInventoryComponent* SlotInventory = SlotWidget->GetInventory();
	const int32 Index = SlotWidget->GetIndex();

	if (!SlotInventory || !SlotInventory->GetSlots().IsValidIndex(Index))
	{
		return;
	}

	FocusInventory = SlotInventory;
	FocusIndex = Index;

	if (IsHolding())
	{
		UFactoryInventoryComponent* HeldInventory = Held.Inventory.Get();
		const bool bSameSlot = HeldInventory == SlotInventory && Held.Index == Index;

		// one item at a time; on the own slot this puts one back
		if (ClickType == EFactorySlotClick::Secondary)
		{
			Held.Count -= bSameSlot ? 1 : HeldInventory->MoveTo(Held.Index, SlotInventory, Index, 1);
		}
		else if (bSameSlot)
		{
			Held.Count = 0;
		}
		else
		{
			const int32 Moved = HeldInventory->MoveTo(Held.Index, SlotInventory, Index, Held.Count);

			// a full target keeps the rest on the cursor, a swap ends the move
			Held.Count = Moved > 0 && HeldInventory->GetSlots()[Held.Index].ItemId == Held.ItemId ? Held.Count - Moved : (Moved > 0 ? 0 : Held.Count);
		}

		if (Held.Count <= 0)
		{
			Held = FHeldStack();
		}

		ValidateHeld();
		Refresh();
		return;
	}

	const FFactoryItemStack Stack = SlotInventory->GetSlots()[Index];

	if (Stack.IsEmpty())
	{
		Refresh();
		return;
	}

	switch (ClickType)
	{
	case EFactorySlotClick::QuickMove:
		SlotInventory->TransferStack(Index, GetOtherInventory(SlotInventory));
		break;

	case EFactorySlotClick::MoveAll:
		SlotInventory->TransferAll(Stack.ItemId, GetOtherInventory(SlotInventory));
		break;

	case EFactorySlotClick::Secondary:
		Held = { SlotInventory, Index, Stack.ItemId, (Stack.Count + 1) / 2 };
		break;

	default:
		Held = { SlotInventory, Index, Stack.ItemId, Stack.Count };
		break;
	}

	Refresh();
}

void UFactoryInventoryWidget::HandleSlotDrop(UFactoryInventorySlotWidget* SlotWidget, const UFactoryItemDragOperation* Operation)
{
	UFactoryInventoryComponent* SourceInventory = Operation->SourceInventory.Get();
	UFactoryInventoryComponent* TargetInventory = SlotWidget->GetInventory();

	Held = FHeldStack();
	FocusInventory = TargetInventory;
	FocusIndex = SlotWidget->GetIndex();

	// the slot may have changed while dragging, e.g. by picking something up
	if (SourceInventory && TargetInventory && SourceInventory->GetSlots().IsValidIndex(Operation->SourceIndex) && SourceInventory->GetSlots()[Operation->SourceIndex].ItemId == Operation->ItemId)
	{
		SourceInventory->MoveTo(Operation->SourceIndex, TargetInventory, SlotWidget->GetIndex(), Operation->Count);
	}

	Refresh();
}

void UFactoryInventoryWidget::ShowDetails(UFactoryInventorySlotWidget* SlotWidget)
{
	if (!DetailsTitle || !DetailsInfo || !DetailsDescription)
	{
		return;
	}

	FName ItemId;
	int32 Count = 0;

	if (SlotWidget && SlotWidget->GetInventory() && SlotWidget->GetInventory()->GetSlots().IsValidIndex(SlotWidget->GetIndex()))
	{
		const FFactoryItemStack& Stack = SlotWidget->GetInventory()->GetSlots()[SlotWidget->GetIndex()];
		ItemId = Stack.ItemId;
		Count = Stack.Count;
	}

	// nothing under the cursor: show what is held
	if (ItemId.IsNone() && IsHolding())
	{
		ItemId = Held.ItemId;
		Count = Held.Count;
	}

	const UFactoryItemDatabase* Database = UFactoryItemDatabase::Get(this);
	const FFactoryItemDefinition* Item = Database && !ItemId.IsNone() ? Database->FindItem(ItemId) : nullptr;

	if (ItemId.IsNone() || Count <= 0)
	{
		DetailsTitle->SetText(FText::GetEmpty());
		DetailsInfo->SetText(FText::GetEmpty());
		DetailsDescription->SetText(FText::GetEmpty());
		return;
	}

	DetailsTitle->SetText(Item ? Item->GetDisplayName() : FactoryLoc::Get("Inv_Unknown"));

	const int32 MaxStack = Item ? Item->GetMaxStackSize() : 0;
	const int32 Total = Inventory.IsValid() ? Inventory->GetItemCount(ItemId) : 0;
	FText Info = FactoryLoc::Format("Inv_StackInfo", { FText::AsNumber(Count), FText::AsNumber(MaxStack), FText::AsNumber(Total) });

	if (Item)
	{
		Info = FText::Format(FText::FromString(TEXT("{0}   ·   {1}")), FactoryLoc::Format("Inv_Tier", { FText::AsNumber(Item->Tier) }), Info);
	}

	DetailsInfo->SetText(Info);
	DetailsDescription->SetText(Item ? Item->GetDescription() : FText::GetEmpty());
}

void UFactoryInventoryWidget::UpdateHint()
{
	if (HintText)
	{
		HintText->SetText(FactoryLoc::Get(bUsingGamepad ? "Inv_HintGamepad" : "Inv_HintMouse"));
	}
}

void UFactoryInventoryWidget::UpdateHeldPreviewPosition()
{
	if (HeldPreviewSlot)
	{
		HeldPreviewSlot->SetPosition(CursorPosition + HeldPreviewOffset);
	}
}

UWidget* UFactoryInventoryWidget::BuildContent()
{
	SlotWidgets.Reset();
	HeldPreviewSlot = nullptr;

	UOverlay* Root = NewWidget<UOverlay>();

	if (UOverlaySlot* TintSlot = Root->AddChildToOverlay(MakeColorBlock(FLinearColor(0.0f, 0.0f, 0.0f, 0.45f))))
	{
		TintSlot->SetHorizontalAlignment(HAlign_Fill);
		TintSlot->SetVerticalAlignment(VAlign_Fill);
	}

	UVerticalBox* Content = NewWidget<UVerticalBox>();

	AddRow(Content, MakeText(FactoryLoc::Get("Inv_Header"), 30.0f, TextColor, true), FMargin(0.0f));
	AddRow(Content, MakeColorBlock(AccentColor, 0.0f, 3.0f), FMargin(0.0f, 8.0f, 0.0f, 18.0f));

	UHorizontalBox* Columns = NewWidget<UHorizontalBox>();

	// equipment column
	UVerticalBox* EquipmentColumn = NewWidget<UVerticalBox>();
	AddRow(EquipmentColumn, MakeText(FactoryLoc::Get("Inv_Equipment"), 14.0f, SubtleTextColor), FMargin(0.0f, 0.0f, 0.0f, 8.0f));

	if (Equipment.IsValid())
	{
		for (int32 Index = 0; Index < Equipment->GetNumSlots(); ++Index)
		{
			AddRow(EquipmentColumn, BuildSlot(Equipment.Get(), Index), FMargin(0.0f, 0.0f, 0.0f, SlotSpacing));
		}
	}

	if (UHorizontalBoxSlot* EquipmentSlot = Columns->AddChildToHorizontalBox(EquipmentColumn))
	{
		EquipmentSlot->SetPadding(FMargin(0.0f, 0.0f, ColumnGap, 0.0f));
		EquipmentSlot->SetVerticalAlignment(VAlign_Top);
	}

	// inventory grid
	UVerticalBox* InventoryColumn = NewWidget<UVerticalBox>();
	const int32 NumSlots = Inventory.IsValid() ? Inventory->GetNumSlots() : 0;
	const int32 NumUsed = Inventory.IsValid() ? Inventory->GetNumUsedSlots() : 0;

	AddRow(InventoryColumn, MakeText(FactoryLoc::Format("Inv_SlotsUsed", { FText::AsNumber(NumUsed), FText::AsNumber(NumSlots) }), 14.0f, NumUsed >= NumSlots ? ErrorColor : SubtleTextColor), FMargin(0.0f, 0.0f, 0.0f, 8.0f));

	UVerticalBox* Grid = NewWidget<UVerticalBox>();

	for (int32 RowStart = 0; RowStart < NumSlots; RowStart += NumColumns)
	{
		UHorizontalBox* Row = NewWidget<UHorizontalBox>();

		for (int32 Index = RowStart; Index < FMath::Min(RowStart + NumColumns, NumSlots); ++Index)
		{
			AddColumn(Row, BuildSlot(Inventory.Get(), Index), false, FMargin(0.0f, 0.0f, SlotSpacing, 0.0f));
		}

		AddRow(Grid, Row, FMargin(0.0f, 0.0f, 0.0f, SlotSpacing));
	}

	UScrollBox* GridScroll = NewWidget<UScrollBox>();
	GridScroll->AddChild(Grid);

	USizeBox* GridSize = NewWidget<USizeBox>();
	GridSize->SetMaxDesiredHeight(MaxGridHeight);
	GridSize->SetContent(GridScroll);
	AddRow(InventoryColumn, GridSize, FMargin(0.0f));

	if (UHorizontalBoxSlot* InventorySlot = Columns->AddChildToHorizontalBox(InventoryColumn))
	{
		InventorySlot->SetVerticalAlignment(VAlign_Top);
	}

	AddRow(Content, Columns, FMargin(0.0f));
	AddRow(Content, BuildDetails(), FMargin(0.0f, 14.0f, 0.0f, 0.0f));

	// controls hint
	HintText = MakeText(FText::GetEmpty(), 13.0f, SubtleTextColor);
	HintText->SetAutoWrapText(true);
	UpdateHint();
	AddRow(Content, HintText, FMargin(0.0f, 10.0f, 0.0f, 0.0f));

	// buttons
	UHorizontalBox* Buttons = NewWidget<UHorizontalBox>();

	AddColumn(Buttons, MakeTextButton(FactoryLoc::Get("Inv_Sort"), [this]()
	{
		if (Inventory.IsValid())
		{
			Held = FHeldStack();
			Inventory->Sort();
		}
	}, EFactoryButtonType::Normal, 17.0f, ETextJustify::Center), true);

	AddColumn(Buttons, MakeTextButton(FactoryLoc::Get("Inv_Close"), [this]()
	{
		OnCloseRequested.ExecuteIfBound();
	}, EFactoryButtonType::Primary, 17.0f, ETextJustify::Center), true, FMargin(0.0f));

	AddRow(Content, Buttons, FMargin(0.0f, 16.0f, 0.0f, 0.0f));

	const float PanelWidth = UFactoryInventorySlotWidget::SlotSize + ColumnGap + NumColumns * SlotPitch + 64.0f + 4.0f;

	if (UOverlaySlot* PanelSlot = Root->AddChildToOverlay(MakeSized(MakePanel(Content), PanelWidth)))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
		PanelSlot->SetVerticalAlignment(VAlign_Center);
	}

	// cursor layer with the held stack, only for the mouse. With a gamepad the held slot is highlighted instead
	if (IsHolding() && !bUsingGamepad)
	{
		UCanvasPanel* CursorLayer = NewWidget<UCanvasPanel>();
		CursorLayer->SetVisibility(ESlateVisibility::HitTestInvisible);

		UFactoryInventorySlotWidget* Preview = NewWidget<UFactoryInventorySlotWidget>();
		Preview->SetupPreview(Held.ItemId, Held.Count);

		HeldPreviewSlot = CursorLayer->AddChildToCanvas(Preview);
		HeldPreviewSlot->SetAutoSize(true);
		UpdateHeldPreviewPosition();

		if (UOverlaySlot* LayerSlot = Root->AddChildToOverlay(CursorLayer))
		{
			LayerSlot->SetHorizontalAlignment(HAlign_Fill);
			LayerSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}

	return Root;
}

UWidget* UFactoryInventoryWidget::BuildSlot(UFactoryInventoryComponent* SlotInventory, int32 Index)
{
	UFactoryInventorySlotWidget* SlotWidget = NewWidget<UFactoryInventorySlotWidget>();
	SlotWidget->Setup(this, SlotInventory, Index);
	SlotWidgets.Add(SlotWidget);
	return SlotWidget;
}

UWidget* UFactoryInventoryWidget::BuildDetails()
{
	UVerticalBox* Details = NewWidget<UVerticalBox>();

	UHorizontalBox* Title = NewWidget<UHorizontalBox>();
	DetailsTitle = MakeText(FText::GetEmpty(), 18.0f, TextColor, true);
	AddColumn(Title, DetailsTitle, true);

	DetailsInfo = MakeText(FText::GetEmpty(), 14.0f, AccentColor, true);
	AddColumn(Title, DetailsInfo, false, FMargin(0.0f));
	AddRow(Details, Title, FMargin(0.0f));

	DetailsDescription = MakeText(FText::GetEmpty(), 14.0f, SubtleTextColor);
	DetailsDescription->SetAutoWrapText(true);
	AddRow(Details, DetailsDescription, FMargin(0.0f, 2.0f, 0.0f, 0.0f));

	return MakeSized(Details, 0.0f, 52.0f);
}
