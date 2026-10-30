// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryMachineWidget.h"
#include "FactoryInventorySlotWidget.h"
#include "FactoryMachine.h"
#include "FactoryProductionComponent.h"
#include "FactoryBuildingDatabase.h"
#include "FactoryRecipeDatabase.h"
#include "FactoryItemDatabase.h"
#include "FactoryInventoryComponent.h"
#include "FactoryGameCharacter.h"
#include "FactoryGamePlayerController.h"
#include "FactoryLocalization.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Brushes/SlateRoundedBoxBrush.h"

namespace
{
	const float MachinePanelWidth = 640.0f;
	const float MachineProgressWidth = 260.0f;
	const float MachineRecipeListHeight = 220.0f;

	/** The panel closes when the player walks further away than this, in cm */
	const double MachinePanelMaxDistance = 1500.0;

	FLinearColor GetPanelStateColor(EFactoryMachineState State)
	{
		// dieselben Farben wie der Graybox-Würfel: grau, gelb, grün, rot
		switch (State)
		{
		case EFactoryMachineState::WaitingForInput:	return FLinearColor(FColor(235, 200, 70));
		case EFactoryMachineState::Working:			return FLinearColor(FColor(110, 205, 110));
		case EFactoryMachineState::OutputFull:		return UFactoryMenuWidget::ErrorColor;
		default:									return UFactoryMenuWidget::SubtleTextColor;
		}
	}
}

void UFactoryMachineWidget::SetMachine(AFactoryMachine* InMachine)
{
	Machine = InMachine;
	FocusKey = NAME_None;
}

UFactoryInventoryComponent* UFactoryMachineWidget::GetPlayerInventory() const
{
	const AFactoryGameCharacter* Character = Cast<AFactoryGameCharacter>(GetOwningPlayerPawn());
	return Character ? Character->GetInventory() : nullptr;
}

void UFactoryMachineWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Refresh();
}

void UFactoryMachineWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const AFactoryMachine* CurrentMachine = Machine.Get();
	const APawn* Pawn = GetOwningPlayerPawn();

	// abgerissen oder zu weit weg: Fenster schließen
	if (!CurrentMachine || !Pawn || FVector::Dist(Pawn->GetActorLocation(), CurrentMachine->GetActorLocation()) > MachinePanelMaxDistance)
	{
		OnCloseRequested.ExecuteIfBound();
		return;
	}

	if (BuildSignature() != ShownSignature)
	{
		Refresh();
	}
	else
	{
		UpdateStatus();
	}
}

FReply UFactoryMachineWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key.IsGamepadKey() && !bUsingGamepad)
	{
		bUsingGamepad = true;
		UpdateHint();
	}

	if (Key == EKeys::E || Key == EKeys::Gamepad_FaceButton_Right)
	{
		OnCloseRequested.ExecuteIfBound();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UFactoryMachineWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// ignore the tiny moves some gamepads/drivers report so the hints don't flicker
	if (bUsingGamepad && !InMouseEvent.GetCursorDelta().IsNearlyZero(2.0f))
	{
		bUsingGamepad = false;
		UpdateHint();
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

bool UFactoryMachineWidget::HandleEscape()
{
	OnCloseRequested.ExecuteIfBound();
	return true;
}

FString UFactoryMachineWidget::BuildSignature() const
{
	const AFactoryMachine* CurrentMachine = Machine.Get();
	const UFactoryProductionComponent* Production = CurrentMachine ? CurrentMachine->GetProduction() : nullptr;
	const UFactoryInventoryComponent* Inventory = GetPlayerInventory();

	if (!Production)
	{
		return FString();
	}

	// Rezept, Puffer und die Inventar-Mengen der Puffer-Items: ändert sich davon etwas, wird neu gebaut
	FString Signature = Production->GetRecipeId().ToString();

	for (const TArray<FFactoryItemStack>* Buffers : { &Production->GetInputs(), &Production->GetOutputs() })
	{
		for (const FFactoryItemStack& Stack : *Buffers)
		{
			Signature += FString::Printf(TEXT("|%s:%d:%d"), *Stack.ItemId.ToString(), Stack.Count, Inventory ? Inventory->GetItemCount(Stack.ItemId) : 0);
		}
	}

	return Signature;
}

void UFactoryMachineWidget::Refresh()
{
	// den fokussierten Button merken, damit er nach dem Neuaufbau wieder den Fokus bekommt
	for (const TPair<FName, TObjectPtr<UButton>>& Entry : FocusButtons)
	{
		if (Entry.Value && Entry.Value->HasKeyboardFocus())
		{
			FocusKey = Entry.Key;
		}
	}

	ShownSignature = BuildSignature();
	RebuildMenu();
	FocusCurrentButton();
}

void UFactoryMachineWidget::FocusCurrentButton()
{
	if (!IsInViewport())
	{
		return;
	}

	// the focus has to stay inside the panel, otherwise the close keys stop working
	TObjectPtr<UButton>* Button = FocusButtons.Find(FocusKey);

	if (!Button || !*Button)
	{
		Button = FocusButtons.Find(TEXT("Close"));
	}

	if (Button && *Button)
	{
		(*Button)->SetKeyboardFocus();
	}
}

UButton* UFactoryMachineWidget::MakeFocusButton(FName Key, UWidget* Content, TFunction<void()> OnClicked, EFactoryButtonType Type, const FMargin& ContentPadding)
{
	UButton* Button = MakeButton(Content, [this, Key, OnClicked = MoveTemp(OnClicked)]()
	{
		FocusKey = Key;
		OnClicked();
	}, Type, ContentPadding);

	FocusButtons.Add(Key, Button);
	return Button;
}

void UFactoryMachineWidget::SelectRecipe(FName RecipeId)
{
	AFactoryMachine* CurrentMachine = Machine.Get();
	AFactoryGamePlayerController* PlayerController = Cast<AFactoryGamePlayerController>(GetOwningPlayer());

	if (!CurrentMachine || !PlayerController || CurrentMachine->GetProduction()->GetRecipeId() == RecipeId)
	{
		return;
	}

	// Puffer und Zutaten eines laufenden Durchlaufs gehen ins Inventar, der Rest in eine Abriss-Kiste
	TArray<FFactoryItemStack> Returned;

	if (CurrentMachine->GetProduction()->SetRecipe(RecipeId, Returned))
	{
		PlayerController->GiveItemsOrCrate(Returned, CurrentMachine->GetActorLocation());
	}

	Refresh();
}

void UFactoryMachineWidget::FillInput(int32 Index)
{
	const AFactoryMachine* CurrentMachine = Machine.Get();
	UFactoryInventoryComponent* Inventory = GetPlayerInventory();

	if (!CurrentMachine || !Inventory || !CurrentMachine->GetProduction()->GetInputs().IsValidIndex(Index))
	{
		return;
	}

	// so viel, wie in den Puffer passt und im Inventar liegt
	UFactoryProductionComponent* Production = CurrentMachine->GetProduction();
	const FName ItemId = Production->GetInputs()[Index].ItemId;
	const int32 Count = FMath::Min(Production->GetInputSpace(ItemId), Inventory->GetItemCount(ItemId));

	if (Count > 0)
	{
		Inventory->RemoveItem(ItemId, Production->InsertInput(ItemId, Count));
	}

	Refresh();
}

void UFactoryMachineWidget::EmptyInput(int32 Index)
{
	const AFactoryMachine* CurrentMachine = Machine.Get();
	UFactoryInventoryComponent* Inventory = GetPlayerInventory();

	if (!CurrentMachine || !Inventory || !CurrentMachine->GetProduction()->GetInputs().IsValidIndex(Index))
	{
		return;
	}

	// nur so viel, wie ins Inventar passt, der Rest bleibt in der Maschine
	UFactoryProductionComponent* Production = CurrentMachine->GetProduction();
	const FFactoryItemStack& Input = Production->GetInputs()[Index];
	const FFactoryItemStack Taken = Production->ExtractInput(Input.ItemId, Inventory->GetFreeSpaceFor(Input.ItemId));

	if (Taken.Count > 0)
	{
		Inventory->AddItem(Taken.ItemId, Taken.Count);
	}

	Refresh();
}

void UFactoryMachineWidget::TakeOutput(int32 Index)
{
	const AFactoryMachine* CurrentMachine = Machine.Get();
	UFactoryInventoryComponent* Inventory = GetPlayerInventory();

	if (!CurrentMachine || !Inventory || !CurrentMachine->GetProduction()->GetOutputs().IsValidIndex(Index))
	{
		return;
	}

	UFactoryProductionComponent* Production = CurrentMachine->GetProduction();
	const FName ItemId = Production->GetOutputs()[Index].ItemId;
	const FFactoryItemStack Taken = Production->ExtractOutput(ItemId, Inventory->GetFreeSpaceFor(ItemId));

	if (Taken.Count > 0)
	{
		Inventory->AddItem(Taken.ItemId, Taken.Count);
	}

	Refresh();
}

void UFactoryMachineWidget::UpdateStatus()
{
	const AFactoryMachine* CurrentMachine = Machine.Get();
	const UFactoryProductionComponent* Production = CurrentMachine ? CurrentMachine->GetProduction() : nullptr;

	if (!Production || !StatusText || !ProgressFill)
	{
		return;
	}

	const EFactoryMachineState State = Production->GetState();
	const float Progress = Production->GetDisplayProgress();

	switch (State)
	{
	case EFactoryMachineState::WaitingForInput:	StatusText->SetText(FactoryLoc::Get("Machine_WaitingForInput")); break;
	case EFactoryMachineState::Working:			StatusText->SetText(FactoryLoc::Format("Machine_Working", { FMath::FloorToInt32(Progress * 100.0f) })); break;
	case EFactoryMachineState::OutputFull:		StatusText->SetText(FactoryLoc::Get("Machine_OutputFull")); break;
	default:									StatusText->SetText(FactoryLoc::Get("Machine_NoRecipe")); break;
	}

	StatusText->SetColorAndOpacity(FSlateColor(GetPanelStateColor(State)));
	ProgressFill->SetWidthOverride(FMath::Max(1.0f, MachineProgressWidth * FMath::Clamp(Progress, 0.0f, 1.0f)));
}

void UFactoryMachineWidget::UpdateHint()
{
	if (HintText)
	{
		HintText->SetText(FactoryLoc::Get(bUsingGamepad ? "MachinePanel_HintGamepad" : "MachinePanel_HintMouse"));
	}
}

UWidget* UFactoryMachineWidget::BuildRecipes()
{
	const AFactoryMachine* CurrentMachine = Machine.Get();
	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);
	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);
	UVerticalBox* List = NewWidget<UVerticalBox>();

	if (!CurrentMachine || !Recipes)
	{
		return List;
	}

	const UFactoryProductionComponent* Production = CurrentMachine->GetProduction();

	auto DescribeStacks = [Items](const TArray<FFactoryItemStack>& Stacks)
	{
		TArray<FString> Parts;

		for (const FFactoryItemStack& Stack : Stacks)
		{
			Parts.Add(FString::Printf(TEXT("%d %s"), Stack.Count, *(Items ? Items->GetDisplayName(Stack.ItemId) : FText::FromName(Stack.ItemId)).ToString()));
		}

		return FString::Join(Parts, TEXT(" + "));
	};

	// eine Zeile pro Rezept: Name links, "1 Eisenerz → 1 Eisenbarren · 30/min" rechts
	for (const FName& RecipeId : Recipes->GetRecipesForBuilding(Production->GetBuildingId()))
	{
		const FFactoryRecipeDefinition* Recipe = Recipes->FindRecipe(RecipeId);
		const TArray<FFactoryItemStack> Outputs = Recipe->GetOutputs();

		UHorizontalBox* Row = NewWidget<UHorizontalBox>();
		UTextBlock* NameText = MakeText(Recipes->GetDisplayName(RecipeId), 16.0f, TextColor, true);
		NameText->SetColorAndOpacity(FSlateColor::UseForeground());
		AddColumn(Row, NameText, true);

		const FString Flow = FString::Printf(TEXT("%s  →  %s   ·   %s"), *DescribeStacks(Recipe->GetInputs()), *DescribeStacks(Outputs),
			*FactoryLoc::Format("MachinePanel_PerMinute", { FText::AsNumber(Outputs.Num() > 0 ? Recipe->GetPerMinute(Outputs[0].Count) : 0.0f) }).ToString());

		UTextBlock* FlowText = MakeText(FText::FromString(Flow), 13.0f, SubtleTextColor);
		AddColumn(Row, FlowText, false, FMargin(0.0f));

		const EFactoryButtonType Type = Production->GetRecipeId() == RecipeId ? EFactoryButtonType::Selected : EFactoryButtonType::Normal;
		AddRow(List, MakeFocusButton(RecipeId, Row, [this, RecipeId]() { SelectRecipe(RecipeId); }, Type, FMargin(14.0f, 9.0f)), FMargin(0.0f, 0.0f, 0.0f, 5.0f));
	}

	UScrollBox* Scroll = NewWidget<UScrollBox>();
	Scroll->AddChild(List);

	USizeBox* ScrollSize = NewWidget<USizeBox>();
	ScrollSize->SetMaxDesiredHeight(MachineRecipeListHeight);
	ScrollSize->SetContent(Scroll);
	return ScrollSize;
}

UWidget* UFactoryMachineWidget::BuildMiningInfo()
{
	const UFactoryProductionComponent* Production = Machine.IsValid() ? Machine->GetProduction() : nullptr;
	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);
	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);
	const FFactoryRecipeDefinition* Recipe = Production && Recipes ? Recipes->FindRecipe(Production->GetRecipeId()) : nullptr;
	const TArray<FFactoryItemStack> Outputs = Recipe ? Recipe->GetOutputs() : TArray<FFactoryItemStack>();

	if (Outputs.IsEmpty())
	{
		return MakeText(FactoryLoc::Get("MachinePanel_NoNode"), 15.0f, ErrorColor);
	}

	// "Baut Eisenerz ab · 120/min (200 % durch die Reinheit)"
	const float PerMinute = Recipe->GetPerMinute(Outputs[0].Count) * Production->GetSpeedPercent() / 100.0f;
	return MakeText(FactoryLoc::Format("MachinePanel_Mining", { Items ? Items->GetDisplayName(Outputs[0].ItemId) : FText::FromName(Outputs[0].ItemId),
		FText::AsNumber(PerMinute), Production->GetSpeedPercent() }), 15.0f, TextColor);
}

UWidget* UFactoryMachineWidget::BuildBufferSlot(int32 Index, bool bInput)
{
	const UFactoryProductionComponent* Production = Machine.IsValid() ? Machine->GetProduction() : nullptr;
	const TArray<FFactoryItemStack>& Buffers = bInput ? Production->GetInputs() : Production->GetOutputs();
	const FFactoryItemStack& Stack = Buffers[Index];
	const UFactoryInventoryComponent* Inventory = GetPlayerInventory();

	UVerticalBox* Column = NewWidget<UVerticalBox>();

	// das Slot-Bild ist ein Inventar-Slot im Vorschau-Modus; leere Puffer zeigen das Item als blassen Platzhalter
	UFactoryInventorySlotWidget* SlotVisual = NewWidget<UFactoryInventorySlotWidget>();
	SlotVisual->SetupPreview(Stack.ItemId, Stack.Count);

	const FName Key(*FString::Printf(TEXT("%s%d"), bInput ? TEXT("In") : TEXT("Out"), Index));
	AddRow(Column, MakeFocusButton(Key, SlotVisual, [this, Index, bInput]()
	{
		if (bInput)
		{
			FillInput(Index);
		}
		else
		{
			TakeOutput(Index);
		}
	}, EFactoryButtonType::Normal, FMargin(4.0f)), FMargin(0.0f));

	const int32 InInventory = Inventory ? Inventory->GetItemCount(Stack.ItemId) : 0;
	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);
	const FText ItemName = Items ? Items->GetDisplayName(Stack.ItemId) : FText::FromName(Stack.ItemId);
	UTextBlock* InventoryText = MakeText(FactoryLoc::Format("MachinePanel_InInventory", { ItemName, InInventory }), 11.0f, SubtleTextColor);
	InventoryText->SetAutoWrapText(true);
	InventoryText->SetJustification(ETextJustify::Center);
	AddRow(Column, InventoryText, FMargin(0.0f, 4.0f, 0.0f, 0.0f));

	if (bInput)
	{
		const FName BackKey(*FString::Printf(TEXT("Back%d"), Index));
		UTextBlock* BackText = MakeText(FactoryLoc::Get("MachinePanel_TakeBack"), 11.0f);
		BackText->SetColorAndOpacity(FSlateColor::UseForeground());
		BackText->SetJustification(ETextJustify::Center);
		AddRow(Column, MakeFocusButton(BackKey, BackText, [this, Index]() { EmptyInput(Index); }, EFactoryButtonType::Normal, FMargin(4.0f, 3.0f)), FMargin(0.0f, 4.0f, 0.0f, 0.0f));
	}

	return MakeSized(Column, UFactoryInventorySlotWidget::SlotSize + 24.0f);
}

UWidget* UFactoryMachineWidget::BuildBuffers()
{
	const UFactoryProductionComponent* Production = Machine.IsValid() ? Machine->GetProduction() : nullptr;
	UHorizontalBox* Row = NewWidget<UHorizontalBox>();

	if (!Production || Production->GetRecipeId().IsNone())
	{
		AddColumn(Row, MakeText(FactoryLoc::Get("MachinePanel_PickRecipe"), 15.0f, SubtleTextColor));
		return Row;
	}

	// Eingänge → Ausgänge
	UVerticalBox* InputColumn = NewWidget<UVerticalBox>();
	AddRow(InputColumn, MakeText(FactoryLoc::Get("MachinePanel_Inputs"), 14.0f, SubtleTextColor), FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	UHorizontalBox* Inputs = NewWidget<UHorizontalBox>();

	for (int32 Index = 0; Index < Production->GetInputs().Num(); ++Index)
	{
		AddColumn(Inputs, BuildBufferSlot(Index, true), false, FMargin(0.0f, 0.0f, 10.0f, 0.0f));
	}

	AddRow(InputColumn, Inputs, FMargin(0.0f));

	if (UHorizontalBoxSlot* InputSlot = Row->AddChildToHorizontalBox(InputColumn))
	{
		InputSlot->SetVerticalAlignment(VAlign_Top);
	}

	AddColumn(Row, MakeText(FText::FromString(TEXT("→")), 30.0f, AccentColor, true), false, FMargin(12.0f, 0.0f, 22.0f, 30.0f));

	UVerticalBox* OutputColumn = NewWidget<UVerticalBox>();
	AddRow(OutputColumn, MakeText(FactoryLoc::Get("MachinePanel_Outputs"), 14.0f, SubtleTextColor), FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	UHorizontalBox* Outputs = NewWidget<UHorizontalBox>();

	for (int32 Index = 0; Index < Production->GetOutputs().Num(); ++Index)
	{
		AddColumn(Outputs, BuildBufferSlot(Index, false), false, FMargin(0.0f, 0.0f, 10.0f, 0.0f));
	}

	AddRow(OutputColumn, Outputs, FMargin(0.0f));

	if (UHorizontalBoxSlot* OutputSlot = Row->AddChildToHorizontalBox(OutputColumn))
	{
		OutputSlot->SetVerticalAlignment(VAlign_Top);
	}

	return Row;
}

UWidget* UFactoryMachineWidget::BuildContent()
{
	FocusButtons.Reset();
	StatusText = nullptr;
	ProgressFill = nullptr;

	UOverlay* Root = NewWidget<UOverlay>();

	if (UOverlaySlot* TintSlot = Root->AddChildToOverlay(MakeColorBlock(FLinearColor(0.0f, 0.0f, 0.0f, 0.35f))))
	{
		TintSlot->SetHorizontalAlignment(HAlign_Fill);
		TintSlot->SetVerticalAlignment(VAlign_Fill);
	}

	const AFactoryMachine* CurrentMachine = Machine.Get();

	if (!CurrentMachine)
	{
		return Root;
	}

	const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this);
	const FName BuildingId = CurrentMachine->GetProduction()->GetBuildingId();

	UVerticalBox* Content = NewWidget<UVerticalBox>();
	AddRow(Content, MakeText(Buildings ? Buildings->GetDisplayName(BuildingId) : FText::FromName(BuildingId), 30.0f, TextColor, true), FMargin(0.0f));
	AddRow(Content, MakeColorBlock(AccentColor, 0.0f, 3.0f), FMargin(0.0f, 8.0f, 0.0f, 14.0f));

	// Zustand + Fortschrittsbalken
	UHorizontalBox* StatusRow = NewWidget<UHorizontalBox>();
	StatusText = MakeText(FText::GetEmpty(), 17.0f, TextColor, true);
	AddColumn(StatusRow, StatusText, true);

	UOverlay* Bar = NewWidget<UOverlay>();

	if (UOverlaySlot* BackSlot = Bar->AddChildToOverlay(MakeColorBlock(FLinearColor(FColor(40, 42, 50)), MachineProgressWidth, 10.0f)))
	{
		BackSlot->SetHorizontalAlignment(HAlign_Left);
	}

	ProgressFill = NewWidget<USizeBox>();
	ProgressFill->SetHeightOverride(10.0f);
	ProgressFill->SetContent(MakeColorBlock(AccentColor));

	if (UOverlaySlot* FillSlot = Bar->AddChildToOverlay(ProgressFill))
	{
		FillSlot->SetHorizontalAlignment(HAlign_Left);
	}

	AddColumn(StatusRow, Bar, false, FMargin(0.0f));
	AddRow(Content, StatusRow, FMargin(0.0f, 0.0f, 0.0f, 16.0f));

	AddRow(Content, BuildBuffers(), FMargin(0.0f, 0.0f, 0.0f, 18.0f));

	// Bohrer: das Rezept kommt vom Vorkommen und lässt sich nicht wechseln
	const FFactoryBuildingDefinition* Building = Buildings ? Buildings->FindBuilding(BuildingId) : nullptr;

	if (Building && Building->Type == EFactoryBuildingType::Drill)
	{
		AddRow(Content, BuildMiningInfo(), FMargin(0.0f));
	}
	else
	{
		AddRow(Content, MakeText(FactoryLoc::Get("MachinePanel_Recipe"), 14.0f, SubtleTextColor), FMargin(0.0f, 0.0f, 0.0f, 6.0f));
		AddRow(Content, BuildRecipes(), FMargin(0.0f));
	}

	HintText = MakeText(FText::GetEmpty(), 13.0f, SubtleTextColor);
	HintText->SetAutoWrapText(true);
	UpdateHint();
	AddRow(Content, HintText, FMargin(0.0f, 12.0f, 0.0f, 0.0f));

	UTextBlock* CloseText = MakeText(FactoryLoc::Get("Inv_Close"), 17.0f, TextColor, true);
	CloseText->SetColorAndOpacity(FSlateColor::UseForeground());
	CloseText->SetJustification(ETextJustify::Center);
	AddRow(Content, MakeFocusButton(TEXT("Close"), CloseText, [this]() { OnCloseRequested.ExecuteIfBound(); }, EFactoryButtonType::Primary, FMargin(20.0f, 12.0f)), FMargin(0.0f, 14.0f, 0.0f, 0.0f));

	if (UOverlaySlot* PanelSlot = Root->AddChildToOverlay(MakeSized(MakePanel(Content), MachinePanelWidth)))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
		PanelSlot->SetVerticalAlignment(VAlign_Center);
	}

	UpdateStatus();
	return Root;
}
