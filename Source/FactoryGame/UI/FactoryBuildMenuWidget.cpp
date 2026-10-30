// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryBuildMenuWidget.h"
#include "FactoryBuildMenuEntryWidget.h"
#include "FactoryBuildingDatabase.h"
#include "FactoryGamePlayerController.h"
#include "FactoryGameCharacter.h"
#include "FactoryInventoryComponent.h"
#include "FactoryItemDatabase.h"
#include "FactoryLocalization.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/CoreStyle.h"

namespace
{
	const int32 BuildMenuColumns = 5;
	const float BuildMenuTileSpacing = 8.0f;

	/** Grid height before the tiles start scrolling (4 rows) */
	const float BuildMenuGridHeight = 4 * (UFactoryBuildMenuEntryWidget::TileHeight + BuildMenuTileSpacing);

	const float BuildMenuCategoryWidth = 180.0f;
	const float BuildMenuColumnGap = 24.0f;
}

AFactoryGamePlayerController* UFactoryBuildMenuWidget::GetFactoryController() const
{
	return Cast<AFactoryGamePlayerController>(GetOwningPlayer());
}

void UFactoryBuildMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// NativeConstruct läuft bei jedem Öffnen: Suche leeren, gehaltenes Gebäude verwerfen.
	// Die Kategorie bleibt vom letzten Öffnen, solange es sie noch gibt.
	SearchText.Reset();
	HeldBuilding = NAME_None;
	HoveredBuilding = NAME_None;
	FocusHotbarSlot = INDEX_NONE;

	const TArray<FName> Categories = GetVisibleCategories();

	if (!Categories.Contains(CurrentCategory))
	{
		CurrentCategory = Categories.Num() > 0 ? Categories[0] : NAME_None;
		FocusBuilding = NAME_None;
	}

	Refresh();
}

FReply UFactoryBuildMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	const bool bTyping = SearchBox && SearchBox->HasKeyboardFocus();

	if (Key.IsGamepadKey())
	{
		SetUsingGamepad(true);
	}

	// beim Tippen in die Suche gehören Q und die Zahlen zum Suchtext
	if (!bTyping)
	{
		if (Key == EKeys::Q)
		{
			OnCloseRequested.ExecuteIfBound();
			return FReply::Handled();
		}

		if (HandleNumberKey(Key))
		{
			return FReply::Handled();
		}
	}

	if (Key == EKeys::Gamepad_LeftShoulder || Key == EKeys::Gamepad_RightShoulder)
	{
		StepCategory(Key == EKeys::Gamepad_LeftShoulder ? -1 : 1);
		return FReply::Handled();
	}

	// gamepad B drops a held building first, then closes
	if (Key == EKeys::Gamepad_FaceButton_Right)
	{
		HandleEscape();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UFactoryBuildMenuWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// ignore the tiny moves some gamepads/drivers report so the hints don't flicker
	if (bUsingGamepad && !InMouseEvent.GetCursorDelta().IsNearlyZero(2.0f))
	{
		SetUsingGamepad(false);
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

bool UFactoryBuildMenuWidget::HandleEscape()
{
	if (!HeldBuilding.IsNone())
	{
		HeldBuilding = NAME_None;
		Refresh();
	}
	else
	{
		OnCloseRequested.ExecuteIfBound();
	}

	return true;
}

void UFactoryBuildMenuWidget::SetUsingGamepad(bool bInUsingGamepad)
{
	if (bUsingGamepad == bInUsingGamepad)
	{
		return;
	}

	bUsingGamepad = bInUsingGamepad;

	// the focus highlight and the hints depend on the input device
	Refresh();
}

TArray<FName> UFactoryBuildMenuWidget::GetVisibleCategories() const
{
	TArray<FName> Categories;
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);

	if (!Buildings)
	{
		return Categories;
	}

	// leere Kategorien werden ausgeblendet, bis es Gebäude dafür gibt
	for (const FName& Category : FFactoryBuildingDefinition::GetMenuCategories())
	{
		for (const FName& Id : Buildings->GetAllBuildingIds())
		{
			const FFactoryBuildingDefinition* Building = Buildings->FindBuilding(Id);

			if (Building && Building->GetMenuCategory() == Category)
			{
				Categories.Add(Category);
				break;
			}
		}
	}

	return Categories;
}

TArray<FName> UFactoryBuildMenuWidget::GetShownBuildings() const
{
	TArray<FName> Shown;
	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);

	if (!Buildings)
	{
		return Shown;
	}

	const FString Search = SearchText.TrimStartAndEnd();

	// sortiert nach Tier und Tabellenreihenfolge (GetAllBuildingIds)
	for (const FName& Id : Buildings->GetAllBuildingIds())
	{
		const FFactoryBuildingDefinition* Building = Buildings->FindBuilding(Id);

		if (!Building)
		{
			continue;
		}

		// die Suche geht über alle Kategorien und findet beide Sprachen
		const bool bMatches = Search.IsEmpty()
			? Building->GetMenuCategory() == CurrentCategory
			: Building->NameEN.Contains(Search) || Building->NameDE.Contains(Search);

		if (bMatches)
		{
			Shown.Add(Id);
		}
	}

	return Shown;
}

void UFactoryBuildMenuWidget::StepCategory(int32 Direction)
{
	const TArray<FName> Categories = GetVisibleCategories();

	if (Categories.Num() == 0)
	{
		return;
	}

	// LB/RB beenden die Suche und laufen zyklisch durch die Kategorien
	const int32 Current = Categories.IndexOfByKey(CurrentCategory);
	CurrentCategory = Categories[(FMath::Max(Current, 0) + Direction + Categories.Num()) % Categories.Num()];
	SearchText.Reset();
	FocusBuilding = NAME_None;
	FocusHotbarSlot = INDEX_NONE;
	Refresh();
}

void UFactoryBuildMenuWidget::Refresh()
{
	RebuildMenu();
	FocusCurrentEntry();
}

void UFactoryBuildMenuWidget::FocusCurrentEntry()
{
	if (!IsInViewport() || (SearchBox && SearchBox->HasKeyboardFocus()))
	{
		return;
	}

	// the focus has to stay inside the menu, otherwise the close keys stop working
	UFactoryBuildMenuEntryWidget* Entry = FindEntry(FocusBuilding, FocusHotbarSlot);

	if (!Entry)
	{
		Entry = TileWidgets.Num() > 0 ? TileWidgets[0] : (HotbarWidgets.Num() > 0 ? HotbarWidgets[0] : nullptr);
	}

	if (Entry)
	{
		Entry->SetKeyboardFocus();
	}
}

UFactoryBuildMenuEntryWidget* UFactoryBuildMenuWidget::FindEntry(FName BuildingId, int32 HotbarSlot) const
{
	if (HotbarSlot != INDEX_NONE)
	{
		return HotbarWidgets.IsValidIndex(HotbarSlot) ? HotbarWidgets[HotbarSlot] : nullptr;
	}

	for (UFactoryBuildMenuEntryWidget* Tile : TileWidgets)
	{
		if (Tile->GetBuildingId() == BuildingId)
		{
			return Tile;
		}
	}

	return nullptr;
}

void UFactoryBuildMenuWidget::HandleEntryFocused(UFactoryBuildMenuEntryWidget* Entry)
{
	FocusHotbarSlot = Entry->GetHotbarSlot();
	FocusBuilding = Entry->IsHotbarSlot() ? NAME_None : Entry->GetBuildingId();

	// mit der Maus zeigen die Details das Gebäude unter dem Zeiger, nicht den Fokus
	if (bUsingGamepad || HoveredBuilding.IsNone())
	{
		ShowDetails(Entry->GetBuildingId());
	}
}

void UFactoryBuildMenuWidget::HandleEntryHovered(UFactoryBuildMenuEntryWidget* Entry, bool bHovered)
{
	if (!bHovered)
	{
		if (!Entry->IsHotbarSlot() && HoveredBuilding == Entry->GetBuildingId())
		{
			HoveredBuilding = NAME_None;
		}

		return;
	}

	if (!Entry->IsHotbarSlot())
	{
		HoveredBuilding = Entry->GetBuildingId();
	}

	if (!Entry->GetBuildingId().IsNone())
	{
		ShowDetails(Entry->GetBuildingId());
	}
}

void UFactoryBuildMenuWidget::HandleEntryClick(UFactoryBuildMenuEntryWidget* Entry)
{
	AFactoryGamePlayerController* PlayerController = GetFactoryController();

	if (!PlayerController)
	{
		return;
	}

	// mit dem Gamepad aufgenommenes Gebäude: A auf einem Hotbar-Slot legt es dort ab
	if (Entry->IsHotbarSlot() && !HeldBuilding.IsNone())
	{
		FocusHotbarSlot = Entry->GetHotbarSlot();
		AssignHotbarSlot(Entry->GetHotbarSlot(), HeldBuilding);
		HeldBuilding = NAME_None;
		Refresh();
		return;
	}

	// A auf die gehaltene Kachel legt sie zurück
	if (!Entry->IsHotbarSlot() && !HeldBuilding.IsNone() && HeldBuilding == Entry->GetBuildingId())
	{
		HeldBuilding = NAME_None;
		Refresh();
		return;
	}

	const FName BuildingId = Entry->GetBuildingId();

	if (BuildingId.IsNone())
	{
		return;
	}

	// schließt das Menü und startet das Platzieren
	PlayerController->PickBuilding(BuildingId);
}

void UFactoryBuildMenuWidget::HandleEntryClear(UFactoryBuildMenuEntryWidget* Entry)
{
	if (Entry->IsHotbarSlot())
	{
		FocusHotbarSlot = Entry->GetHotbarSlot();
		AssignHotbarSlot(Entry->GetHotbarSlot(), NAME_None);
		Refresh();
	}
}

void UFactoryBuildMenuWidget::HandleEntryHold(UFactoryBuildMenuEntryWidget* Entry)
{
	const FName BuildingId = Entry->GetBuildingId();

	// zweimal Y legt es zurück
	HeldBuilding = HeldBuilding == BuildingId ? NAME_None : BuildingId;
	FocusBuilding = BuildingId;
	FocusHotbarSlot = INDEX_NONE;
	Refresh();
}

void UFactoryBuildMenuWidget::HandleEntryDrop(UFactoryBuildMenuEntryWidget* Entry, const UFactoryBuildingDragOperation* Operation)
{
	const AFactoryGamePlayerController* PlayerController = GetFactoryController();

	if (!PlayerController || !Entry->IsHotbarSlot())
	{
		return;
	}

	const int32 Target = Entry->GetHotbarSlot();

	// von Slot zu Slot: die beiden tauschen
	if (Operation->SourceHotbarSlot != INDEX_NONE && Operation->SourceHotbarSlot != Target)
	{
		const FName TargetBuilding = PlayerController->GetHotbarSlot(Target);
		AssignHotbarSlot(Target, Operation->BuildingId);
		AssignHotbarSlot(Operation->SourceHotbarSlot, TargetBuilding);
	}
	else
	{
		AssignHotbarSlot(Target, Operation->BuildingId);
	}

	HeldBuilding = NAME_None;
	Refresh();
}

bool UFactoryBuildMenuWidget::HandleNumberKey(const FKey& Key)
{
	static const FKey NumberKeys[] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six, EKeys::Seven, EKeys::Eight, EKeys::Nine, EKeys::Zero };

	// die Zahl legt das Gebäude unter der Maus (oder die fokussierte Kachel) auf diesen Slot
	const FName BuildingId = !HoveredBuilding.IsNone() ? HoveredBuilding : FocusBuilding;

	if (BuildingId.IsNone())
	{
		return false;
	}

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(NumberKeys); ++Index)
	{
		if (NumberKeys[Index] == Key)
		{
			AssignHotbarSlot(Index, BuildingId);
			Refresh();
			return true;
		}
	}

	return false;
}

void UFactoryBuildMenuWidget::AssignHotbarSlot(int32 Index, FName BuildingId)
{
	AFactoryGamePlayerController* PlayerController = GetFactoryController();

	if (!PlayerController)
	{
		return;
	}

	// jedes Gebäude liegt höchstens einmal auf der Hotbar, ein alter Platz wird frei
	if (!BuildingId.IsNone())
	{
		for (int32 Other = 0; Other < AFactoryGamePlayerController::NumHotbarSlots; ++Other)
		{
			if (Other != Index && PlayerController->GetHotbarSlot(Other) == BuildingId)
			{
				PlayerController->SetHotbarSlot(Other, NAME_None);
			}
		}
	}

	PlayerController->SetHotbarSlot(Index, BuildingId);
}

void UFactoryBuildMenuWidget::HandleSearchChanged(const FText& Text)
{
	SearchText = Text.ToString();
	RefreshTiles();
}

void UFactoryBuildMenuWidget::HandleSearchCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	SearchText = Text.ToString();

	// Enter springt auf den ersten Treffer, Esc verlässt nur das Suchfeld
	if (CommitMethod == ETextCommit::OnEnter || CommitMethod == ETextCommit::OnCleared)
	{
		FocusBuilding = NAME_None;
		FocusHotbarSlot = INDEX_NONE;

		if (TileWidgets.Num() > 0)
		{
			TileWidgets[0]->SetKeyboardFocus();
		}
		else
		{
			FocusCurrentEntry();
		}
	}
}

void UFactoryBuildMenuWidget::ShowDetails(FName BuildingId)
{
	if (!DetailsBox)
	{
		return;
	}

	DetailsBox->ClearChildren();

	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	const FFactoryBuildingDefinition* Building = Buildings && !BuildingId.IsNone() ? Buildings->FindBuilding(BuildingId) : nullptr;

	if (!Building)
	{
		return;
	}

	// Titel und Eckdaten: "Produktion · Tier 1 · 4 × 4 × 5 m · 1 → 1"
	UHorizontalBox* Title = NewWidget<UHorizontalBox>();
	AddColumn(Title, MakeText(Building->GetDisplayName(), 18.0f, TextColor, true), true);

	FString Info = FString::Printf(TEXT("%s  ·  %s"),
		*FFactoryBuildingDefinition::GetCategoryDisplayName(Building->GetMenuCategory()).ToString(),
		*FactoryLoc::Format("Inv_Tier", { FText::AsNumber(Building->Tier) }).ToString());

	if (Building->IsMachine())
	{
		Info += FString::Printf(TEXT("  ·  %d × %d × %d m  ·  %s"), Building->SizeX, Building->SizeY, Building->SizeZ,
			*FactoryLoc::Format("BuildMenu_Ports", { FText::AsNumber(Building->Inputs), FText::AsNumber(Building->Outputs) }).ToString());
	}

	AddColumn(Title, MakeText(FText::FromString(Info), 14.0f, AccentColor, true), false, FMargin(0.0f));
	AddRow(DetailsBox, Title, FMargin(0.0f));

	// Kosten: "hast / brauchst", rot wenn es nicht reicht. Bänder kosten pro angefangene 2 m
	const AFactoryGameCharacter* Character = Cast<AFactoryGameCharacter>(GetOwningPlayerPawn());
	const UFactoryInventoryComponent* Inventory = Character ? Character->GetInventory() : nullptr;
	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);

	UHorizontalBox* Costs = NewWidget<UHorizontalBox>();
	AddColumn(Costs, MakeText(FactoryLoc::Get(Building->Type == EFactoryBuildingType::Belt ? "BuildMenu_CostPer2m" : "BuildMenu_Cost"), 14.0f, SubtleTextColor), false, FMargin(0.0f, 0.0f, 12.0f, 0.0f));

	for (const FFactoryItemStack& Cost : Building->GetCosts())
	{
		const int32 Have = Inventory ? Inventory->GetItemCount(Cost.ItemId) : 0;
		const FText Name = Items ? Items->GetDisplayName(Cost.ItemId) : FText::FromName(Cost.ItemId);
		AddColumn(Costs, MakeText(FactoryLoc::Format("Build_Cost", { Have, Cost.Count, Name }), 14.0f, Have >= Cost.Count ? TextColor : ErrorColor), false, FMargin(0.0f, 0.0f, 16.0f, 0.0f));
	}

	AddRow(DetailsBox, Costs, FMargin(0.0f, 4.0f, 0.0f, 0.0f));
}

void UFactoryBuildMenuWidget::UpdateHint()
{
	if (!HintText)
	{
		return;
	}

	if (!HeldBuilding.IsNone())
	{
		const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
		HintText->SetText(FactoryLoc::Format("BuildMenu_HintHeld", { Buildings ? Buildings->GetDisplayName(HeldBuilding) : FText::FromName(HeldBuilding) }));
		return;
	}

	HintText->SetText(FactoryLoc::Get(bUsingGamepad ? "BuildMenu_HintGamepad" : "BuildMenu_HintMouse"));
}

void UFactoryBuildMenuWidget::RefreshTiles()
{
	if (!TileGrid)
	{
		return;
	}

	TileGrid->ClearChildren();
	TileWidgets.Reset();

	const TArray<FName> Shown = GetShownBuildings();

	if (Shown.Num() == 0)
	{
		AddRow(TileGrid, MakeText(FactoryLoc::Get(SearchText.TrimStartAndEnd().IsEmpty() ? "BuildMenu_EmptyCategory" : "BuildMenu_NoResults"), 15.0f, SubtleTextColor), FMargin(0.0f, 8.0f));
		return;
	}

	for (int32 RowStart = 0; RowStart < Shown.Num(); RowStart += BuildMenuColumns)
	{
		UHorizontalBox* Row = NewWidget<UHorizontalBox>();

		for (int32 Index = RowStart; Index < FMath::Min(RowStart + BuildMenuColumns, Shown.Num()); ++Index)
		{
			UFactoryBuildMenuEntryWidget* Tile = NewWidget<UFactoryBuildMenuEntryWidget>();
			Tile->SetupTile(this, Shown[Index]);
			TileWidgets.Add(Tile);
			AddColumn(Row, Tile, false, FMargin(0.0f, 0.0f, BuildMenuTileSpacing, 0.0f));
		}

		AddRow(TileGrid, Row, FMargin(0.0f, 0.0f, 0.0f, BuildMenuTileSpacing));
	}
}

UWidget* UFactoryBuildMenuWidget::BuildCategories()
{
	UVerticalBox* List = NewWidget<UVerticalBox>();
	const bool bSearching = !SearchText.TrimStartAndEnd().IsEmpty();

	for (const FName& Category : GetVisibleCategories())
	{
		// während der Suche ist keine Kategorie gewählt; ein Klick beendet die Suche
		const EFactoryButtonType Type = !bSearching && Category == CurrentCategory ? EFactoryButtonType::Selected : EFactoryButtonType::Normal;

		AddRow(List, MakeTextButton(FFactoryBuildingDefinition::GetCategoryDisplayName(Category), [this, Category]()
		{
			CurrentCategory = Category;
			SearchText.Reset();
			FocusBuilding = NAME_None;
			FocusHotbarSlot = INDEX_NONE;
			Refresh();
		}, Type, 16.0f), FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	}

	return MakeSized(List, BuildMenuCategoryWidth);
}

UWidget* UFactoryBuildMenuWidget::BuildHotbar()
{
	HotbarWidgets.Reset();

	UHorizontalBox* Hotbar = NewWidget<UHorizontalBox>();

	for (int32 Index = 0; Index < AFactoryGamePlayerController::NumHotbarSlots; ++Index)
	{
		UFactoryBuildMenuEntryWidget* SlotWidget = NewWidget<UFactoryBuildMenuEntryWidget>();
		SlotWidget->SetupHotbarSlot(this, Index);
		HotbarWidgets.Add(SlotWidget);
		AddColumn(Hotbar, SlotWidget, false, FMargin(0.0f, 0.0f, 6.0f, 0.0f));
	}

	return Hotbar;
}

UWidget* UFactoryBuildMenuWidget::BuildContent()
{
	TileWidgets.Reset();
	HotbarWidgets.Reset();

	UOverlay* Root = NewWidget<UOverlay>();

	if (UOverlaySlot* TintSlot = Root->AddChildToOverlay(MakeColorBlock(FLinearColor(0.0f, 0.0f, 0.0f, 0.45f))))
	{
		TintSlot->SetHorizontalAlignment(HAlign_Fill);
		TintSlot->SetVerticalAlignment(VAlign_Fill);
	}

	UVerticalBox* Content = NewWidget<UVerticalBox>();

	// Kopfzeile: Titel links, Suchfeld rechts
	UHorizontalBox* Header = NewWidget<UHorizontalBox>();
	AddColumn(Header, MakeText(FactoryLoc::Get("BuildMenu_Header"), 30.0f, TextColor, true), true);

	SearchBox = NewWidget<UEditableTextBox>();
	SearchBox->SetText(FText::FromString(SearchText));
	SearchBox->SetHintText(FactoryLoc::Get("BuildMenu_Search"));
	SearchBox->SetRevertTextOnEscape(false);
	SearchBox->SetClearKeyboardFocusOnCommit(false);

	FEditableTextBoxStyle BoxStyle = SearchBox->GetWidgetStyle();
	BoxStyle.SetBackgroundImageNormal(FSlateRoundedBoxBrush(FLinearColor(FColor(30, 32, 38)), 6.0f, FLinearColor(FColor(70, 74, 84)), 1.0f));
	BoxStyle.SetBackgroundImageHovered(FSlateRoundedBoxBrush(FLinearColor(FColor(34, 36, 43)), 6.0f, FLinearColor(FColor(120, 90, 60)), 1.0f));
	BoxStyle.SetBackgroundImageFocused(FSlateRoundedBoxBrush(FLinearColor(FColor(34, 36, 43)), 6.0f, AccentColor, 1.5f));
	BoxStyle.SetBackgroundColor(FLinearColor::White);
	BoxStyle.SetForegroundColor(TextColor);
	BoxStyle.SetFocusedForegroundColor(FLinearColor::White);
	BoxStyle.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 16));
	BoxStyle.SetPadding(FMargin(10.0f, 7.0f));
	SearchBox->SetWidgetStyle(BoxStyle);

	SearchBox->OnTextChanged.AddDynamic(this, &UFactoryBuildMenuWidget::HandleSearchChanged);
	SearchBox->OnTextCommitted.AddDynamic(this, &UFactoryBuildMenuWidget::HandleSearchCommitted);
	AddColumn(Header, MakeSized(SearchBox, 280.0f), false, FMargin(0.0f));

	AddRow(Content, Header, FMargin(0.0f));
	AddRow(Content, MakeColorBlock(AccentColor, 0.0f, 3.0f), FMargin(0.0f, 8.0f, 0.0f, 18.0f));

	// Kategorien links, Kacheln rechts
	UHorizontalBox* Columns = NewWidget<UHorizontalBox>();

	if (UHorizontalBoxSlot* CategorySlot = Columns->AddChildToHorizontalBox(BuildCategories()))
	{
		CategorySlot->SetPadding(FMargin(0.0f, 0.0f, BuildMenuColumnGap, 0.0f));
		CategorySlot->SetVerticalAlignment(VAlign_Top);
	}

	TileGrid = NewWidget<UVerticalBox>();
	RefreshTiles();

	UScrollBox* GridScroll = NewWidget<UScrollBox>();
	GridScroll->AddChild(TileGrid);

	// feste Höhe, damit das Fenster beim Kategoriewechsel nicht springt
	USizeBox* GridSize = NewWidget<USizeBox>();
	GridSize->SetHeightOverride(BuildMenuGridHeight);
	GridSize->SetContent(GridScroll);

	if (UHorizontalBoxSlot* GridSlot = Columns->AddChildToHorizontalBox(GridSize))
	{
		GridSlot->SetVerticalAlignment(VAlign_Top);
		GridSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	AddRow(Content, Columns, FMargin(0.0f));

	// Details zum markierten Gebäude
	DetailsBox = NewWidget<UVerticalBox>();
	AddRow(Content, MakeSized(DetailsBox, 0.0f, 52.0f), FMargin(0.0f, 14.0f, 0.0f, 0.0f));

	// Hotbar zum Belegen
	AddRow(Content, MakeText(FactoryLoc::Get("BuildMenu_Hotbar"), 14.0f, SubtleTextColor), FMargin(0.0f, 10.0f, 0.0f, 6.0f));
	AddRow(Content, BuildHotbar(), FMargin(0.0f));

	HintText = MakeText(FText::GetEmpty(), 13.0f, SubtleTextColor);
	HintText->SetAutoWrapText(true);
	UpdateHint();
	AddRow(Content, HintText, FMargin(0.0f, 12.0f, 0.0f, 0.0f));

	// Breite = Kategorien + Abstand + 5 Kacheln + Innenabstand des Panels (2 x 32) + Reserve für die Scrollleiste
	const float PanelWidth = BuildMenuCategoryWidth + BuildMenuColumnGap + BuildMenuColumns * (UFactoryBuildMenuEntryWidget::TileWidth + BuildMenuTileSpacing) + 64.0f + 16.0f;

	if (UOverlaySlot* PanelSlot = Root->AddChildToOverlay(MakeSized(MakePanel(Content), PanelWidth)))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
		PanelSlot->SetVerticalAlignment(VAlign_Center);
	}

	ShowDetails(!FocusBuilding.IsNone() ? FocusBuilding : (TileWidgets.Num() > 0 ? TileWidgets[0]->GetBuildingId() : NAME_None));

	return Root;
}
