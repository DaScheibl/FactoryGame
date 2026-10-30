// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryBuildMenuEntryWidget.h"
#include "FactoryBuildMenuWidget.h"
#include "FactoryBuildingDatabase.h"
#include "FactoryGamePlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Brushes/SlateRoundedBoxBrush.h"

namespace
{
	const float EntryCornerRadius = 6.0f;
	const FLinearColor EntryColor = FLinearColor(FColor(28, 30, 36, 220));
	const FLinearColor EntryHighlightColor = FLinearColor(FColor(62, 50, 40, 235));
}

void UFactoryBuildMenuEntryWidget::SetupTile(UFactoryBuildMenuWidget* InOwner, FName InBuildingId)
{
	Owner = InOwner;
	BuildingId = InBuildingId;
	HotbarSlot = INDEX_NONE;
	RebuildMenu();
}

void UFactoryBuildMenuEntryWidget::SetupHotbarSlot(UFactoryBuildMenuWidget* InOwner, int32 InHotbarSlot)
{
	Owner = InOwner;
	BuildingId = NAME_None;
	HotbarSlot = InHotbarSlot;
	RebuildMenu();
}

void UFactoryBuildMenuEntryWidget::SetupPreview(FName InBuildingId)
{
	BuildingId = InBuildingId;
	SetVisibility(ESlateVisibility::HitTestInvisible);
	RebuildMenu();
}

FName UFactoryBuildMenuEntryWidget::GetBuildingId() const
{
	if (IsHotbarSlot())
	{
		const AFactoryGamePlayerController* PlayerController = Owner.IsValid() ? Owner->GetFactoryController() : nullptr;
		return PlayerController ? PlayerController->GetHotbarSlot(HotbarSlot) : NAME_None;
	}

	return BuildingId;
}

UWidget* UFactoryBuildMenuEntryWidget::BuildContent()
{
	const FName Id = GetBuildingId();
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	const FFactoryBuildingDefinition* Building = Buildings && !Id.IsNone() ? Buildings->FindBuilding(Id) : nullptr;

	UOverlay* Content = NewWidget<UOverlay>();

	if (IsHotbarSlot())
	{
		// Tastennummer oben links wie in der HUD-Hotbar, darunter der Gebäudename
		UTextBlock* KeyText = MakeText(FText::AsNumber((HotbarSlot + 1) % 10), 12.0f, SubtleTextColor, true);

		if (UOverlaySlot* KeySlot = Content->AddChildToOverlay(KeyText))
		{
			KeySlot->SetHorizontalAlignment(HAlign_Left);
			KeySlot->SetVerticalAlignment(VAlign_Top);
			KeySlot->SetPadding(FMargin(4.0f, 1.0f));
		}

		if (!Id.IsNone())
		{
			UTextBlock* NameText = MakeText(Building ? Building->GetDisplayName() : FText::FromName(Id), 10.0f);
			NameText->SetJustification(ETextJustify::Center);
			NameText->SetAutoWrapText(true);

			if (UOverlaySlot* NameSlot = Content->AddChildToOverlay(NameText))
			{
				NameSlot->SetHorizontalAlignment(HAlign_Fill);
				NameSlot->SetVerticalAlignment(VAlign_Center);
				NameSlot->SetPadding(FMargin(2.0f, 12.0f, 2.0f, 2.0f));
			}
		}
	}
	else
	{
		// Platzhalter bis zu den Gebäude-Icons: Name und Größe als Text
		UVerticalBox* Lines = NewWidget<UVerticalBox>();

		UTextBlock* NameText = MakeText(Building ? Building->GetDisplayName() : FText::FromName(Id), 14.0f, TextColor, true);
		NameText->SetJustification(ETextJustify::Center);
		NameText->SetAutoWrapText(true);
		AddRow(Lines, NameText, FMargin(0.0f));

		if (Building && Building->IsMachine())
		{
			UTextBlock* SizeText = MakeText(FText::FromString(FString::Printf(TEXT("%d × %d × %d m"), Building->SizeX, Building->SizeY, Building->SizeZ)), 11.0f, SubtleTextColor);
			SizeText->SetJustification(ETextJustify::Center);
			AddRow(Lines, SizeText, FMargin(0.0f, 4.0f, 0.0f, 0.0f));
		}

		if (UOverlaySlot* LinesSlot = Content->AddChildToOverlay(Lines))
		{
			LinesSlot->SetHorizontalAlignment(HAlign_Fill);
			LinesSlot->SetVerticalAlignment(VAlign_Center);
			LinesSlot->SetPadding(FMargin(6.0f));
		}
	}

	Background = NewWidget<UBorder>();
	Background->SetPadding(FMargin(2.0f));
	Background->SetContent(Content);
	UpdateBackground();

	return IsHotbarSlot() ? MakeSized(Background, HotbarSlotSize, HotbarSlotSize) : MakeSized(Background, TileWidth, TileHeight);
}

void UFactoryBuildMenuEntryWidget::UpdateBackground()
{
	if (!Background)
	{
		return;
	}

	const bool bHeldHere = !IsHotbarSlot() && Owner.IsValid() && !BuildingId.IsNone() && Owner->GetHeldBuilding() == BuildingId;
	const bool bShowFocus = bFocused && Owner.IsValid() && Owner->IsUsingGamepad();

	if (bHeldHere || bShowFocus)
	{
		Background->SetBrush(FSlateRoundedBoxBrush(EntryHighlightColor, EntryCornerRadius, AccentColor, 2.0f));
	}
	else if (bHovered)
	{
		Background->SetBrush(FSlateRoundedBoxBrush(EntryHighlightColor, EntryCornerRadius, AccentColor, 1.0f));
	}
	else
	{
		Background->SetBrush(FSlateRoundedBoxBrush(EntryColor, EntryCornerRadius, FLinearColor(FColor(255, 255, 255, 20)), 1.0f));
	}
}

FReply UFactoryBuildMenuEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!Owner.IsValid())
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	Owner->SetUsingGamepad(false);

	const FKey Button = InMouseEvent.GetEffectingButton();

	if (Button == EKeys::RightMouseButton && IsHotbarSlot())
	{
		Owner->HandleEntryClear(this);
		return FReply::Handled();
	}

	if (Button != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	// Klick oder Ziehen entscheidet sich erst beim Bewegen bzw. Loslassen (wie im Inventar)
	bLeftPressed = true;
	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

FReply UFactoryBuildMenuEntryWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bLeftPressed)
	{
		bLeftPressed = false;

		if (Owner.IsValid())
		{
			Owner->HandleEntryClick(this);
		}

		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UFactoryBuildMenuEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	bLeftPressed = false;

	const FName Id = GetBuildingId();

	if (!Owner.IsValid() || Id.IsNone())
	{
		return;
	}

	UFactoryBuildingDragOperation* Operation = NewObject<UFactoryBuildingDragOperation>();
	Operation->BuildingId = Id;
	Operation->SourceHotbarSlot = HotbarSlot;

	// das Bild am Mauszeiger ist eine zweite Kachel im Vorschau-Modus
	UFactoryBuildMenuEntryWidget* Visual = CreateWidget<UFactoryBuildMenuEntryWidget>(Owner.Get(), UFactoryBuildMenuEntryWidget::StaticClass());
	Visual->SetupPreview(Id);

	Operation->DefaultDragVisual = Visual;
	Operation->Pivot = EDragPivot::CenterCenter;

	OutOperation = Operation;
}

bool UFactoryBuildMenuEntryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UFactoryBuildingDragOperation* Operation = Cast<UFactoryBuildingDragOperation>(InOperation);

	if (!Operation || !Owner.IsValid() || !IsHotbarSlot())
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	Owner->HandleEntryDrop(this, Operation);
	return true;
}

void UFactoryBuildMenuEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	bHovered = true;
	UpdateBackground();

	if (Owner.IsValid())
	{
		Owner->HandleEntryHovered(this, true);
	}
}

void UFactoryBuildMenuEntryWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	bHovered = false;
	bLeftPressed = false;
	UpdateBackground();

	if (Owner.IsValid())
	{
		Owner->HandleEntryHovered(this, false);
	}
}

void UFactoryBuildMenuEntryWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	bFocused = true;
	UpdateBackground();

	if (Owner.IsValid())
	{
		Owner->HandleEntryFocused(this);
	}
}

void UFactoryBuildMenuEntryWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	bFocused = false;
	UpdateBackground();
}

FReply UFactoryBuildMenuEntryWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (!Owner.IsValid())
	{
		return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	}

	if (Key.IsGamepadKey() && !Owner->IsUsingGamepad())
	{
		Owner->SetUsingGamepad(true);
	}

	// A = Klick, Y = Gebäude für die Hotbar aufnehmen, X = Hotbar-Slot leeren
	if (Key == EKeys::Gamepad_FaceButton_Bottom || Key == EKeys::Enter || Key == EKeys::SpaceBar)
	{
		Owner->HandleEntryClick(this);
		return FReply::Handled();
	}

	if (Key == EKeys::Gamepad_FaceButton_Top && !IsHotbarSlot())
	{
		Owner->HandleEntryHold(this);
		return FReply::Handled();
	}

	if (Key == EKeys::Gamepad_FaceButton_Left && IsHotbarSlot())
	{
		Owner->HandleEntryClear(this);
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
