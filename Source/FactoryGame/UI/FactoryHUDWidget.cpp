// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryHUDWidget.h"
#include "FactoryGamePlayerController.h"
#include "FactoryItemDatabase.h"
#include "FactoryLocalization.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Brushes/SlateRoundedBoxBrush.h"

namespace
{
	const float NotificationDuration = 4.0f;
	const int32 MaxNotifications = 5;
	const float HotbarSlotSize = 64.0f;

	const FLinearColor PositiveColor = FLinearColor(FColor(93, 176, 67));
}

void UFactoryHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(false);
	// HitTestInvisible: Mausklicks gehen durch das HUD hindurch
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UFactoryHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Abgelaufene Einträge entfernen. Neu gebaut wird nur, wenn sich wirklich etwas geändert hat.
	// Echtzeit (FPlatformTime) statt Weltzeit, damit Meldungen auch während der Pause ablaufen.
	const double Now = FPlatformTime::Seconds();
	bool bChanged = Notifications.RemoveAll([Now](const FNotification& Entry) { return Entry.ExpiresAt <= Now; }) > 0;

	if (!HintText.IsEmpty() && HintExpiresAt <= Now)
	{
		HintText = FText::GetEmpty();
		bChanged = true;
	}

	if (bChanged)
	{
		RebuildMenu();
	}
}

void UFactoryHUDWidget::ShowNotification(const FText& Text, EFactoryNotificationType Type)
{
	FNotification& Entry = Notifications.AddDefaulted_GetRef();
	Entry.Text = Text;
	Entry.Type = Type;
	Entry.ExpiresAt = FPlatformTime::Seconds() + NotificationDuration;

	if (Notifications.Num() > MaxNotifications)
	{
		Notifications.RemoveAt(0, Notifications.Num() - MaxNotifications);
	}

	RebuildMenu();
}

void UFactoryHUDWidget::ShowItemNotification(FName ItemId, int32 Delta)
{
	if (Delta == 0)
	{
		return;
	}

	// mehrere "+5 Eisenplatte" kurz hintereinander werden zu "+15 Eisenplatte" zusammengefasst,
	// die Anzeigedauer beginnt dabei neu
	FNotification* Entry = Notifications.FindByPredicate([ItemId, Delta](const FNotification& Existing)
	{
		return Existing.ItemId == ItemId && FMath::Sign(Existing.ItemDelta) == FMath::Sign(Delta);
	});

	if (!Entry)
	{
		Entry = &Notifications.AddDefaulted_GetRef();
		Entry->ItemId = ItemId;
		Entry->Type = Delta > 0 ? EFactoryNotificationType::Positive : EFactoryNotificationType::Info;
	}

	Entry->ItemDelta += Delta;
	Entry->ExpiresAt = FPlatformTime::Seconds() + NotificationDuration;

	const UFactoryItemDatabase* Database = UFactoryItemDatabase::Get(this);
	const FText ItemName = Database ? Database->GetDisplayName(ItemId) : FText::FromName(ItemId);
	Entry->Text = FactoryLoc::Format(Entry->ItemDelta > 0 ? "Notify_ItemsAdded" : "Notify_ItemsRemoved", { FText::AsNumber(FMath::Abs(Entry->ItemDelta)), ItemName });

	if (Notifications.Num() > MaxNotifications)
	{
		Notifications.RemoveAt(0, Notifications.Num() - MaxNotifications);
	}

	RebuildMenu();
}

void UFactoryHUDWidget::ShowHint(const FText& Text, float Duration)
{
	HintText = Text;
	HintExpiresAt = FPlatformTime::Seconds() + Duration;
	RebuildMenu();
}

UWidget* UFactoryHUDWidget::BuildContent()
{
	// Overlay = Ebenen übereinander mit eigener Ausrichtung:
	// Fadenkreuz in der Mitte, Hinweis + Hotbar unten, Meldungen oben rechts
	UOverlay* Root = NewWidget<UOverlay>();

	if (UOverlaySlot* CrosshairSlot = Root->AddChildToOverlay(BuildCrosshair()))
	{
		CrosshairSlot->SetHorizontalAlignment(HAlign_Center);
		CrosshairSlot->SetVerticalAlignment(VAlign_Center);
	}

	// hint line above the hotbar
	UVerticalBox* Bottom = NewWidget<UVerticalBox>();

	if (!HintText.IsEmpty())
	{
		UTextBlock* Hint = MakeText(HintText, 16.0f);
		Hint->SetJustification(ETextJustify::Center);
		AddRow(Bottom, Hint, FMargin(0.0f, 0.0f, 0.0f, 10.0f));
	}

	AddRow(Bottom, BuildHotbar(), FMargin(0.0f));

	if (UOverlaySlot* BottomSlot = Root->AddChildToOverlay(Bottom))
	{
		BottomSlot->SetHorizontalAlignment(HAlign_Center);
		BottomSlot->SetVerticalAlignment(VAlign_Bottom);
		BottomSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 28.0f));
	}

	if (UOverlaySlot* NotificationSlot = Root->AddChildToOverlay(BuildNotifications()))
	{
		NotificationSlot->SetHorizontalAlignment(HAlign_Right);
		NotificationSlot->SetVerticalAlignment(VAlign_Top);
		NotificationSlot->SetPadding(FMargin(0.0f, 32.0f, 32.0f, 0.0f));
	}

	return Root;
}

UWidget* UFactoryHUDWidget::BuildCrosshair()
{
	// a small dot with a dark outline, readable on bright and dark backgrounds
	UBorder* Outline = NewWidget<UBorder>();
	Outline->SetBrush(FSlateRoundedBoxBrush(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f), 4.0f));
	Outline->SetPadding(FMargin(1.5f));

	UBorder* Dot = NewWidget<UBorder>();
	Dot->SetBrush(FSlateRoundedBoxBrush(FLinearColor(1.0f, 1.0f, 1.0f, 0.9f), 2.5f));
	Outline->SetContent(MakeSized(Dot, 5.0f, 5.0f));

	return Outline;
}

UWidget* UFactoryHUDWidget::BuildHotbar()
{
	const AFactoryGamePlayerController* PlayerController = Cast<AFactoryGamePlayerController>(GetOwningPlayer());

	UHorizontalBox* Hotbar = NewWidget<UHorizontalBox>();

	for (int32 Index = 0; Index < AFactoryGamePlayerController::NumHotbarSlots; ++Index)
	{
		const bool bSelected = PlayerController && PlayerController->GetSelectedHotbarSlot() == Index;
		const FName Shortcut = PlayerController ? PlayerController->GetHotbarSlot(Index) : NAME_None;

		UOverlay* SlotContent = NewWidget<UOverlay>();

		// key number in the top left corner, 1-9 and 0 like the keyboard row
		UTextBlock* KeyText = MakeText(FText::AsNumber((Index + 1) % 10), 12.0f, bSelected ? AccentColor : SubtleTextColor, true);

		if (UOverlaySlot* KeySlot = SlotContent->AddChildToOverlay(KeyText))
		{
			KeySlot->SetHorizontalAlignment(HAlign_Left);
			KeySlot->SetVerticalAlignment(VAlign_Top);
			KeySlot->SetPadding(FMargin(6.0f, 3.0f));
		}

		// Platzhalter: der Shortcut-Name als Text, bis es Gebäude-Icons gibt
		if (!Shortcut.IsNone())
		{
			UTextBlock* ShortcutText = MakeText(FText::FromName(Shortcut), 10.0f);
			ShortcutText->SetJustification(ETextJustify::Center);
			ShortcutText->SetAutoWrapText(true);

			if (UOverlaySlot* ShortcutSlot = SlotContent->AddChildToOverlay(ShortcutText))
			{
				ShortcutSlot->SetHorizontalAlignment(HAlign_Fill);
				ShortcutSlot->SetVerticalAlignment(VAlign_Center);
				ShortcutSlot->SetPadding(FMargin(4.0f, 14.0f, 4.0f, 4.0f));
			}
		}

		UBorder* SlotBorder = NewWidget<UBorder>();
		SlotBorder->SetBrush(bSelected
			? FSlateRoundedBoxBrush(FLinearColor(FColor(62, 50, 40, 225)), 6.0f, AccentColor, 2.0f)
			: FSlateRoundedBoxBrush(FLinearColor(FColor(14, 15, 19, 190)), 6.0f, FLinearColor(FColor(255, 255, 255, 25)), 1.0f));
		SlotBorder->SetPadding(FMargin(0.0f));
		SlotBorder->SetContent(SlotContent);

		AddColumn(Hotbar, MakeSized(SlotBorder, HotbarSlotSize, HotbarSlotSize), false, FMargin(3.0f, 0.0f));
	}

	return Hotbar;
}

UWidget* UFactoryHUDWidget::BuildNotifications()
{
	UVerticalBox* List = NewWidget<UVerticalBox>();

	for (const FNotification& Entry : Notifications)
	{
		const FLinearColor StripeColor = Entry.Type == EFactoryNotificationType::Positive ? PositiveColor
			: Entry.Type == EFactoryNotificationType::Warning ? ErrorColor
			: AccentColor;

		UHorizontalBox* Row = NewWidget<UHorizontalBox>();
		AddColumn(Row, MakeColorBlock(StripeColor, 4.0f, 22.0f), false, FMargin(0.0f, 0.0f, 10.0f, 0.0f));
		AddColumn(Row, MakeText(Entry.Text, 16.0f), true, FMargin(0.0f));

		UBorder* Box = NewWidget<UBorder>();
		Box->SetBrush(FSlateRoundedBoxBrush(FLinearColor(FColor(14, 15, 19, 210)), 6.0f));
		Box->SetPadding(FMargin(10.0f, 8.0f, 16.0f, 8.0f));
		Box->SetContent(Row);

		AddRow(List, MakeSized(Box, 300.0f), FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	}

	return List;
}
