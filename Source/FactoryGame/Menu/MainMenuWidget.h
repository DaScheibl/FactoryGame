// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FactoryMenuWidget.h"
#include "GenericPlatform/GenericWindow.h"
#include "MainMenuWidget.generated.h"

class UEditableTextBox;

/** Pages that can be shown next to the main menu buttons */
enum class EMainMenuPage : uint8
{
	None,
	NewWorld,
	LoadWorld,
	Settings
};

/**
 *  Main menu with Continue, New World, Load World, Settings and Quit
 */
UCLASS()
class FACTORYGAME_API UMainMenuWidget : public UFactoryMenuWidget
{
	GENERATED_BODY()

protected:

	/** Builds the menu layout */
	virtual UWidget* BuildContent() override;

	/** Closes the open page */
	virtual bool HandleEscape() override;

private:

	UWidget* BuildNavigation();
	UWidget* BuildNewWorldPage();
	UWidget* BuildLoadWorldPage();
	UWidget* BuildSettingsPage();

	/** Adds the status line and the given buttons to a page */
	void AddPageFooter(class UVerticalBox* Page, const TArray<UWidget*>& Buttons);

	/** Opens the given page, or closes it if it's already open */
	void TogglePage(EMainMenuPage Page);

	/** Shows a message below the page buttons */
	void SetStatus(const FText& Message, bool bIsError);

	/** Rebuilds the layout while keeping the entered world name */
	void Refresh();

	/** Copies the current user settings into the pending settings */
	void ReadSettings();

	/** Applies and saves the pending settings */
	void ApplySettings();

	void CreateWorld();
	void LoadSelectedWorld();
	void DeleteSelectedWorld();

	/** Currently shown page */
	EMainMenuPage ActivePage = EMainMenuPage::None;

	/** Status line below the page buttons */
	FText StatusMessage;
	bool bStatusIsError = false;

	/** Name text box of the new world page */
	UPROPERTY()
	TObjectPtr<UEditableTextBox> WorldNameBox;

	/** Entered world name, kept across rebuilds */
	FString PendingWorldName;

	/** Slot selected in the load world list */
	FString SelectedWorldSlot;

	/** True after the first click on delete */
	bool bConfirmDelete = false;

	/** Settings edited on the settings page, applied with the apply button */
	int32 PendingLanguage = 0;
	int32 PendingWindowMode = 0;
	int32 PendingResolution = 0;
	int32 PendingQuality = 3;
	bool bPendingVSync = false;

	/** Resolutions offered on the settings page */
	TArray<FIntPoint> Resolutions;
};
