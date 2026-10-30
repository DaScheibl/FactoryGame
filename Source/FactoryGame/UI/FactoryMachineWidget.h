// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FactoryMenuWidget.h"
#include "FactoryMachineWidget.generated.h"

class AFactoryMachine;
class USizeBox;
class UFactoryInventoryComponent;

/**
 *  Machine panel (E on a machine): recipe list, input and output buffers, state and progress.
 *  Input buffers are filled from the player inventory with a click and emptied with the button below them,
 *  output buffers are taken with a click. The game keeps running while the panel is open.
 *
 *  Aufbau wie die anderen Fenster (UFactoryMenuWidget). Zustand und Fortschritt werden jedes Bild direkt
 *  in die Textfelder geschrieben; ändern sich Puffer oder Inventar, baut sich das Fenster neu auf
 *  und behält dabei den Fokus (FocusKey). Regeln: Docs/Standards/14-maschinen-panel.md
 */
UCLASS()
class FACTORYGAME_API UFactoryMachineWidget : public UFactoryMenuWidget
{
	GENERATED_BODY()

public:

	/** Called when the player wants to close the panel */
	FSimpleDelegate OnCloseRequested;

	/** Sets the machine to show */
	void SetMachine(AFactoryMachine* InMachine);

	AFactoryMachine* GetMachine() const { return Machine.Get(); }

	/** Gives the keyboard/gamepad focus to the last used button */
	void FocusCurrentButton();

protected:

	virtual void NativeConstruct() override;

	/** Updates state and progress, rebuilds when buffers or inventory changed, closes when the machine is gone */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Closes with E, gamepad B */
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual bool HandleEscape() override;

	virtual UWidget* BuildContent() override;

private:

	UWidget* BuildRecipes();

	/** Drills: what and how fast they mine, instead of the recipe list */
	UWidget* BuildMiningInfo();
	UWidget* BuildBuffers();

	/** One buffer slot as a button, plus the item count in the inventory and, for inputs, a take-back button */
	UWidget* BuildBufferSlot(int32 Index, bool bInput);

	/** Creates a button and remembers it under a key, so the focus survives a rebuild */
	UButton* MakeFocusButton(FName Key, UWidget* Content, TFunction<void()> OnClicked, EFactoryButtonType Type, const FMargin& ContentPadding);

	/** Switches to another recipe, everything inside goes to the inventory */
	void SelectRecipe(FName RecipeId);

	/** Fills an input buffer from the inventory */
	void FillInput(int32 Index);

	/** Takes an input buffer back into the inventory */
	void EmptyInput(int32 Index);

	/** Takes an output buffer into the inventory */
	void TakeOutput(int32 Index);

	/** Rebuilds the panel and keeps the focus */
	void Refresh();

	/** Buffers and inventory counts joined, to detect changes cheaply */
	FString BuildSignature() const;

	void UpdateStatus();

	/** Power line: use or output, network load, fuse */
	FText GetPowerText(FLinearColor& OutColor) const;

	/** Switches the tripped fuse of the generator's network on again */
	void ResetFuse();

	void UpdateHint();

	UFactoryInventoryComponent* GetPlayerInventory() const;

	TWeakObjectPtr<AFactoryMachine> Machine;

	/** Signature of the shown buffers */
	FString ShownSignature;

	/** Button that gets the focus after a rebuild */
	FName FocusKey;

	bool bUsingGamepad = false;

	UPROPERTY()
	TMap<FName, TObjectPtr<UButton>> FocusButtons;

	UPROPERTY()
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY()
	TObjectPtr<USizeBox> ProgressFill;

	UPROPERTY()
	TObjectPtr<UTextBlock> HintText;

	UPROPERTY()
	TObjectPtr<UTextBlock> PowerText;
};
