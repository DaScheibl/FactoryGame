// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryHubWidget.h"
#include "FactoryHub.h"
#include "FactoryRecipeDatabase.h"
#include "FactoryItemDatabase.h"
#include "FactoryBuildingDatabase.h"
#include "FactoryInventoryComponent.h"
#include "FactoryGameCharacter.h"
#include "FactoryGamePlayerController.h"
#include "FactoryHUDWidget.h"
#include "FactoryCrate.h"
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

namespace
{
	const float HubPanelWidth = 760.0f;
	const float HubRecipeListWidth = 300.0f;
	const float HubListHeight = 320.0f;
	const float HubProgressWidth = 360.0f;

	/** The window closes when the player walks further away than this, in cm */
	const double HubMaxDistance = 2000.0;

	FText DescribeItem(const UFactoryItemDatabase* Items, const FFactoryItemStack& Stack)
	{
		return FText::FromString(FString::Printf(TEXT("%d %s"), Stack.Count, *(Items ? Items->GetDisplayName(Stack.ItemId) : FText::FromName(Stack.ItemId)).ToString()));
	}
}

void UFactoryHubWidget::SetHub(AFactoryHub* InHub)
{
	Hub = InHub;
}

UFactoryInventoryComponent* UFactoryHubWidget::GetPlayerInventory() const
{
	const AFactoryGameCharacter* Character = Cast<AFactoryGameCharacter>(GetOwningPlayerPawn());
	return Character ? Character->GetInventory() : nullptr;
}

void UFactoryHubWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CraftProgress = 0.0f;
	RebuildMenu();
	FocusFirstButton();
}

void UFactoryHubWidget::FocusFirstButton()
{
	// the focus has to stay inside the window, otherwise the close keys stop working
	if (IsInViewport() && FirstButton)
	{
		FirstButton->SetKeyboardFocus();
	}
}

FReply UFactoryHubWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::E || Key == EKeys::Gamepad_FaceButton_Right)
	{
		OnCloseRequested.ExecuteIfBound();
		return FReply::Handled();
	}

	if (Key == EKeys::Gamepad_LeftShoulder || Key == EKeys::Gamepad_RightShoulder)
	{
		Tab = Tab == ETab::Workbench ? ETab::Milestones : ETab::Workbench;
		RebuildMenu();
		FocusFirstButton();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

bool UFactoryHubWidget::HandleEscape()
{
	OnCloseRequested.ExecuteIfBound();
	return true;
}

float UFactoryHubWidget::GetCraftTime() const
{
	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);
	const FFactoryRecipeDefinition* Recipe = Recipes && !SelectedRecipe.IsNone() ? Recipes->FindRecipe(SelectedRecipe) : nullptr;
	return Recipe ? FMath::Max(0.1f, Recipe->Duration * HandCraftFactor) : 0.0f;
}

bool UFactoryHubWidget::HasIngredients() const
{
	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);
	const FFactoryRecipeDefinition* Recipe = Recipes && !SelectedRecipe.IsNone() ? Recipes->FindRecipe(SelectedRecipe) : nullptr;
	const UFactoryInventoryComponent* Inventory = GetPlayerInventory();

	if (!Recipe || !Inventory)
	{
		return false;
	}

	return !Recipe->GetInputs().ContainsByPredicate([Inventory](const FFactoryItemStack& Input) { return Inventory->GetItemCount(Input.ItemId) < Input.Count; });
}

void UFactoryHubWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const AFactoryHub* CurrentHub = Hub.Get();
	const APawn* Pawn = GetOwningPlayerPawn();

	if (!CurrentHub || !Pawn || FVector::Dist(Pawn->GetActorLocation(), CurrentHub->GetActorLocation()) > HubMaxDistance)
	{
		OnCloseRequested.ExecuteIfBound();
		return;
	}

	if (Tab != ETab::Workbench)
	{
		return;
	}

	// Herstellen, solange der Button gedrückt gehalten wird (Maus oder Gamepad A); loslassen setzt zurück
	if (CraftButton && CraftButton->IsPressed() && HasIngredients())
	{
		CraftProgress += InDeltaTime;

		if (CraftProgress >= GetCraftTime())
		{
			CraftProgress = 0.0f;
			Craft();
		}
	}
	else
	{
		CraftProgress = 0.0f;
	}

	UpdateWorkbenchTexts();
}

void UFactoryHubWidget::Craft()
{
	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);
	const FFactoryRecipeDefinition* Recipe = Recipes ? Recipes->FindRecipe(SelectedRecipe) : nullptr;
	UFactoryInventoryComponent* Inventory = GetPlayerInventory();
	AFactoryGamePlayerController* PlayerController = Cast<AFactoryGamePlayerController>(GetOwningPlayer());
	AFactoryHub* CurrentHub = Hub.Get();

	if (!Recipe || !Inventory || !PlayerController || !CurrentHub || !HasIngredients())
	{
		return;
	}

	for (const FFactoryItemStack& Input : Recipe->GetInputs())
	{
		Inventory->RemoveItem(Input.ItemId, Input.Count);
	}

	// Ergebnis ins Inventar; was nicht passt, ins HUB-Lager, und was dort nicht passt, in eine Abriss-Kiste
	TArray<FFactoryItemStack> Leftovers;

	for (const FFactoryItemStack& Output : Recipe->GetOutputs())
	{
		int32 Rest = Output.Count - PlayerController->GiveItems(Output.ItemId, Output.Count);
		Rest -= Rest > 0 ? CurrentHub->GetStorage()->AddItem(Output.ItemId, Rest) : 0;

		if (Rest > 0)
		{
			Leftovers.Emplace(Output.ItemId, Rest);
		}
	}

	if (Leftovers.Num() > 0)
	{
		PlayerController->GiveItemsOrCrate(Leftovers, CurrentHub->GetActorLocation() + FVector(0.0, -500.0, 0.0));
	}
}

void UFactoryHubWidget::UpdateWorkbenchTexts()
{
	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);
	const FFactoryRecipeDefinition* Recipe = Recipes && !SelectedRecipe.IsNone() ? Recipes->FindRecipe(SelectedRecipe) : nullptr;
	const UFactoryInventoryComponent* Inventory = GetPlayerInventory();
	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);

	if (!Recipe)
	{
		return;
	}

	// "3 / 5 Eisenbarren", rot wenn es fehlt
	const TArray<FFactoryItemStack> Inputs = Recipe->GetInputs();

	for (int32 Index = 0; Index < Inputs.Num() && Index < IngredientTexts.Num(); ++Index)
	{
		const int32 Have = Inventory ? Inventory->GetItemCount(Inputs[Index].ItemId) : 0;
		const FText Name = Items ? Items->GetDisplayName(Inputs[Index].ItemId) : FText::FromName(Inputs[Index].ItemId);
		IngredientTexts[Index]->SetText(FactoryLoc::Format("Build_Cost", { Have, Inputs[Index].Count, Name }));
		IngredientTexts[Index]->SetColorAndOpacity(FSlateColor(Have >= Inputs[Index].Count ? TextColor : ErrorColor));
	}

	const float CraftTime = GetCraftTime();

	if (CraftStatusText)
	{
		CraftStatusText->SetText(!HasIngredients() ? FactoryLoc::Get("Hub_MissingIngredients")
			: CraftProgress > 0.0f ? FactoryLoc::Format("Hub_Crafting", { FMath::FloorToInt32(CraftProgress / CraftTime * 100.0f) })
			: FactoryLoc::Format("Hub_CraftTime", { FText::AsNumber(CraftTime) }));
		CraftStatusText->SetColorAndOpacity(FSlateColor(HasIngredients() ? SubtleTextColor : ErrorColor));
	}

	if (ProgressFill)
	{
		ProgressFill->SetWidthOverride(FMath::Max(1.0f, HubProgressWidth * FMath::Clamp(CraftTime > 0.0f ? CraftProgress / CraftTime : 0.0f, 0.0f, 1.0f)));
	}
}

UWidget* UFactoryHubWidget::BuildTabs()
{
	UHorizontalBox* Tabs = NewWidget<UHorizontalBox>();

	auto AddTab = [this, Tabs](FName LabelKey, ETab TabValue)
	{
		UButton* Button = MakeTextButton(FactoryLoc::Get(LabelKey), [this, TabValue]()
		{
			Tab = TabValue;
			RebuildMenu();
			FocusFirstButton();
		}, Tab == TabValue ? EFactoryButtonType::Selected : EFactoryButtonType::Normal, 16.0f, ETextJustify::Center);

		AddColumn(Tabs, MakeSized(Button, 170.0f), false, FMargin(0.0f, 0.0f, 8.0f, 0.0f));
	};

	AddTab("Hub_TabWorkbench", ETab::Workbench);
	AddTab("Hub_TabMilestones", ETab::Milestones);

	// das Lager öffnet das Inventarfenster mit dem HUB-Lager darüber
	AddColumn(Tabs, MakeSized(MakeTextButton(FactoryLoc::Get("Hub_OpenStorage"), [this]() { OnStorageRequested.ExecuteIfBound(); },
		EFactoryButtonType::Normal, 16.0f, ETextJustify::Center), 170.0f), false, FMargin(0.0f));

	return Tabs;
}

UWidget* UFactoryHubWidget::BuildWorkbench()
{
	IngredientTexts.Reset();
	CraftButton = nullptr;
	CraftStatusText = nullptr;
	ProgressFill = nullptr;

	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);
	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);
	UHorizontalBox* Columns = NewWidget<UHorizontalBox>();

	if (!Recipes)
	{
		return Columns;
	}

	// links: alle Werkbank-Rezepte; gesperrte stehen grau mit "ab Tier N" darunter
	UVerticalBox* List = NewWidget<UVerticalBox>();
	const TArray<FName> RecipeIds = Recipes->GetRecipesForBuilding(UFactoryRecipeDatabase::WorkbenchId);

	if (SelectedRecipe.IsNone() || !RecipeIds.Contains(SelectedRecipe))
	{
		const FName* FirstUnlocked = RecipeIds.FindByPredicate([this, Recipes](const FName& Id) { return AFactoryHub::IsRecipeUnlocked(this, *Recipes->FindRecipe(Id), true); });
		SelectedRecipe = FirstUnlocked ? *FirstUnlocked : NAME_None;
	}

	for (const FName& RecipeId : RecipeIds)
	{
		const FFactoryRecipeDefinition* Recipe = Recipes->FindRecipe(RecipeId);
		const bool bUnlocked = AFactoryHub::IsRecipeUnlocked(this, *Recipe, true);
		const FText Label = bUnlocked ? Recipes->GetDisplayName(RecipeId)
			: FText::Format(FText::FromString(TEXT("{0}  ({1})")), Recipes->GetDisplayName(RecipeId), FactoryLoc::Format("Common_FromTier", { Recipe->Tier }));

		UButton* Button = MakeTextButton(Label, [this, RecipeId]()
		{
			SelectedRecipe = RecipeId;
			CraftProgress = 0.0f;
			RebuildMenu();

			if (CraftButton)
			{
				CraftButton->SetKeyboardFocus();
			}
		}, RecipeId == SelectedRecipe ? EFactoryButtonType::Selected : EFactoryButtonType::Normal, 15.0f);

		Button->SetIsEnabled(bUnlocked);
		AddRow(List, Button, FMargin(0.0f, 0.0f, 0.0f, 5.0f));

		if (!FirstButton && bUnlocked)
		{
			FirstButton = Button;
		}
	}

	UScrollBox* Scroll = NewWidget<UScrollBox>();
	Scroll->AddChild(List);

	USizeBox* ListSize = NewWidget<USizeBox>();
	ListSize->SetWidthOverride(HubRecipeListWidth);
	ListSize->SetHeightOverride(HubListHeight);
	ListSize->SetContent(Scroll);
	AddColumn(Columns, ListSize, false, FMargin(0.0f, 0.0f, 24.0f, 0.0f));

	// rechts: Zutaten, Ergebnis und der Herstellen-Button
	UVerticalBox* Details = NewWidget<UVerticalBox>();
	const FFactoryRecipeDefinition* Recipe = !SelectedRecipe.IsNone() ? Recipes->FindRecipe(SelectedRecipe) : nullptr;

	if (Recipe)
	{
		AddRow(Details, MakeText(Recipes->GetDisplayName(SelectedRecipe), 20.0f, TextColor, true), FMargin(0.0f, 0.0f, 0.0f, 10.0f));
		AddRow(Details, MakeText(FactoryLoc::Get("Hub_Ingredients"), 14.0f, SubtleTextColor), FMargin(0.0f, 0.0f, 0.0f, 4.0f));

		for (int32 Index = 0; Index < Recipe->GetInputs().Num(); ++Index)
		{
			UTextBlock* Line = MakeText(FText::GetEmpty(), 15.0f);
			IngredientTexts.Add(Line);
			AddRow(Details, Line, FMargin(8.0f, 0.0f, 0.0f, 2.0f));
		}

		AddRow(Details, MakeText(FactoryLoc::Get("Hub_Result"), 14.0f, SubtleTextColor), FMargin(0.0f, 10.0f, 0.0f, 4.0f));

		for (const FFactoryItemStack& Output : Recipe->GetOutputs())
		{
			AddRow(Details, MakeText(DescribeItem(Items, Output), 15.0f, AccentColor, true), FMargin(8.0f, 0.0f, 0.0f, 2.0f));
		}

		// Fortschrittsbalken
		UOverlay* Bar = NewWidget<UOverlay>();

		if (UOverlaySlot* BackSlot = Bar->AddChildToOverlay(MakeColorBlock(FLinearColor(FColor(40, 42, 50)), HubProgressWidth, 10.0f)))
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

		AddRow(Details, Bar, FMargin(0.0f, 18.0f, 0.0f, 6.0f));

		CraftStatusText = MakeText(FText::GetEmpty(), 13.0f, SubtleTextColor);
		AddRow(Details, CraftStatusText, FMargin(0.0f, 0.0f, 0.0f, 10.0f));

		// der Klick selbst tut nichts, hergestellt wird in NativeTick, solange der Button gedrückt ist
		CraftButton = MakeTextButton(FactoryLoc::Get("Hub_CraftHold"), []() {}, EFactoryButtonType::Primary, 17.0f, ETextJustify::Center);
		AddRow(Details, MakeSized(CraftButton, HubProgressWidth), FMargin(0.0f));

		if (!FirstButton)
		{
			FirstButton = CraftButton;
		}
	}
	else
	{
		AddRow(Details, MakeText(FactoryLoc::Get("Hub_NoRecipes"), 15.0f, SubtleTextColor));
	}

	AddColumn(Columns, Details, true, FMargin(0.0f));
	UpdateWorkbenchTexts();
	return Columns;
}

UWidget* UFactoryHubWidget::BuildMilestones()
{
	AFactoryHub* CurrentHub = Hub.Get();
	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);
	const UFactoryInventoryComponent* Inventory = GetPlayerInventory();
	UVerticalBox* Box = NewWidget<UVerticalBox>();

	const FFactoryMilestone* Milestone = CurrentHub ? AFactoryHub::GetMilestone(CurrentHub->GetTier() + 1) : nullptr;

	if (!Milestone)
	{
		AddRow(Box, MakeText(FactoryLoc::Get("Hub_AllMilestones"), 16.0f, SubtleTextColor));
		return MakeSized(Box, 0.0f, HubListHeight);
	}

	AddRow(Box, MakeText(FactoryLoc::Format("Hub_NextTier", { Milestone->Tier }), 20.0f, TextColor, true), FMargin(0.0f, 0.0f, 0.0f, 10.0f));

	// pro Paket: "abgegeben / benötigt", dahinter wie viele im Inventar liegen
	for (const FFactoryItemStack& Cost : Milestone->Costs)
	{
		const int32 Delivered = CurrentHub->GetDelivered(Cost.ItemId);
		const FText Name = Items ? Items->GetDisplayName(Cost.ItemId) : FText::FromName(Cost.ItemId);

		UHorizontalBox* Row = NewWidget<UHorizontalBox>();
		AddColumn(Row, MakeText(FactoryLoc::Format("Build_Cost", { Delivered, Cost.Count, Name }), 16.0f, Delivered >= Cost.Count ? FLinearColor(FColor(110, 205, 110)) : TextColor), true);
		AddColumn(Row, MakeText(FactoryLoc::Format("Hub_InInventory", { Inventory ? Inventory->GetItemCount(Cost.ItemId) : 0 }), 13.0f, SubtleTextColor), false, FMargin(0.0f));
		AddRow(Box, Row, FMargin(8.0f, 0.0f, 0.0f, 4.0f));
	}

	// was die Stufe freischaltet: alle Gebäude mit genau dieser Tier
	TArray<FString> Unlocks;

	if (const UFactoryBuildingDatabase* Buildings = UFactoryBuildingDatabase::Get(this))
	{
		for (const FName& Id : Buildings->GetAllBuildingIds())
		{
			const FFactoryBuildingDefinition* Building = Buildings->FindBuilding(Id);

			if (Building && Building->Tier == Milestone->Tier && Building->Type != EFactoryBuildingType::Hub)
			{
				Unlocks.Add(Building->GetDisplayName().ToString());
			}
		}
	}

	if (Unlocks.Num() > 0)
	{
		UTextBlock* UnlockText = MakeText(FactoryLoc::Format("Hub_Unlocks", { FText::FromString(FString::Join(Unlocks, TEXT(", "))) }), 14.0f, SubtleTextColor);
		UnlockText->SetAutoWrapText(true);
		AddRow(Box, UnlockText, FMargin(0.0f, 10.0f, 0.0f, 0.0f));
	}

	UHorizontalBox* Buttons = NewWidget<UHorizontalBox>();

	UButton* DeliverButton = MakeTextButton(FactoryLoc::Get("Hub_Deliver"), [this]()
	{
		if (AFactoryHub* TargetHub = Hub.Get())
		{
			TargetHub->Deliver(GetPlayerInventory());
		}

		RebuildMenu();
		FocusFirstButton();
	}, EFactoryButtonType::Normal, 16.0f, ETextJustify::Center);

	FirstButton = DeliverButton;
	AddColumn(Buttons, DeliverButton, true);

	UButton* UnlockButton = MakeTextButton(FactoryLoc::Format("Hub_Unlock", { Milestone->Tier }), [this]()
	{
		AFactoryHub* TargetHub = Hub.Get();

		if (TargetHub && TargetHub->UnlockNext())
		{
			if (const AFactoryGamePlayerController* PlayerController = Cast<AFactoryGamePlayerController>(GetOwningPlayer()))
			{
				if (UFactoryHUDWidget* HUD = PlayerController->GetHUDWidget())
				{
					HUD->ShowNotification(FactoryLoc::Format("Hub_Unlocked", { TargetHub->GetTier() }), EFactoryNotificationType::Positive);
				}
			}
		}

		RebuildMenu();
		FocusFirstButton();
	}, EFactoryButtonType::Primary, 16.0f, ETextJustify::Center);

	UnlockButton->SetIsEnabled(CurrentHub->CanUnlockNext());
	AddColumn(Buttons, UnlockButton, true, FMargin(0.0f));

	AddRow(Box, Buttons, FMargin(0.0f, 20.0f, 0.0f, 0.0f));
	return MakeSized(Box, 0.0f, HubListHeight);
}

UWidget* UFactoryHubWidget::BuildContent()
{
	FirstButton = nullptr;

	UOverlay* Root = NewWidget<UOverlay>();

	if (UOverlaySlot* TintSlot = Root->AddChildToOverlay(MakeColorBlock(FLinearColor(0.0f, 0.0f, 0.0f, 0.35f))))
	{
		TintSlot->SetHorizontalAlignment(HAlign_Fill);
		TintSlot->SetVerticalAlignment(VAlign_Fill);
	}

	const AFactoryHub* CurrentHub = Hub.Get();

	if (!CurrentHub)
	{
		return Root;
	}

	UVerticalBox* Content = NewWidget<UVerticalBox>();
	AddRow(Content, MakeText(FactoryLoc::Format("Hub_Header", { CurrentHub->GetTier() }), 30.0f, TextColor, true), FMargin(0.0f));
	AddRow(Content, MakeColorBlock(AccentColor, 0.0f, 3.0f), FMargin(0.0f, 8.0f, 0.0f, 14.0f));
	AddRow(Content, BuildTabs(), FMargin(0.0f, 0.0f, 0.0f, 18.0f));
	AddRow(Content, Tab == ETab::Workbench ? BuildWorkbench() : BuildMilestones(), FMargin(0.0f));

	UTextBlock* Hint = MakeText(FactoryLoc::Get("Hub_Hint"), 13.0f, SubtleTextColor);
	Hint->SetAutoWrapText(true);
	AddRow(Content, Hint, FMargin(0.0f, 14.0f, 0.0f, 0.0f));

	if (UOverlaySlot* PanelSlot = Root->AddChildToOverlay(MakeSized(MakePanel(Content), HubPanelWidth)))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
		PanelSlot->SetVerticalAlignment(VAlign_Center);
	}

	return Root;
}
