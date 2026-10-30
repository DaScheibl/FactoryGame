// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FactoryMenuWidget.h"
#include "FactoryHubWidget.generated.h"

class AFactoryHub;
class USizeBox;
class UVerticalBox;
class UFactoryInventoryComponent;

/**
 *  HUB window (E on the HUB): workbench and milestones, plus a button that opens the HUB storage next to the inventory.
 *  Workbench: pick a recipe, then hold the craft button; every craft time one result is made from the inventory.
 *
 *  Aufbau wie die anderen Fenster (UFactoryMenuWidget). Während der Herstellen-Button gehalten wird,
 *  darf das Fenster nicht neu gebaut werden (sonst wäre der Button weg), deshalb aktualisiert NativeTick
 *  die Mengen und den Fortschritt direkt in den Textfeldern. Regeln: Docs/Standards/16-hub.md
 */
UCLASS()
class FACTORYGAME_API UFactoryHubWidget : public UFactoryMenuWidget
{
	GENERATED_BODY()

public:

	/** Hand crafting takes this many times the recipe duration */
	static constexpr float HandCraftFactor = 2.0f;

	/** Called when the player wants to close the window */
	FSimpleDelegate OnCloseRequested;

	/** Called when the player wants to see the HUB storage */
	FSimpleDelegate OnStorageRequested;

	void SetHub(AFactoryHub* InHub);

	/** Gives the keyboard/gamepad focus to a button of the window */
	void FocusFirstButton();

protected:

	virtual void NativeConstruct() override;

	/** Crafts while the craft button is held and updates amounts and progress */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Closes with E, gamepad B, switches tabs with LB/RB */
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	virtual bool HandleEscape() override;

	virtual UWidget* BuildContent() override;

private:

	enum class ETab : uint8
	{
		Workbench,
		Milestones
	};

	UWidget* BuildTabs();
	UWidget* BuildWorkbench();
	UWidget* BuildMilestones();

	/** Takes the ingredients of the selected recipe and gives the result */
	void Craft();

	/** True if the inventory holds all ingredients of the selected recipe */
	bool HasIngredients() const;

	/** Writes the ingredient amounts (have / need) and the progress */
	void UpdateWorkbenchTexts();

	/** Hand craft time of the selected recipe in seconds */
	float GetCraftTime() const;

	UFactoryInventoryComponent* GetPlayerInventory() const;

	TWeakObjectPtr<AFactoryHub> Hub;

	ETab Tab = ETab::Workbench;

	FName SelectedRecipe;

	float CraftProgress = 0.0f;

	UPROPERTY()
	TObjectPtr<UButton> CraftButton;

	UPROPERTY()
	TObjectPtr<UButton> FirstButton;

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> IngredientTexts;

	UPROPERTY()
	TObjectPtr<UTextBlock> CraftStatusText;

	UPROPERTY()
	TObjectPtr<USizeBox> ProgressFill;
};
