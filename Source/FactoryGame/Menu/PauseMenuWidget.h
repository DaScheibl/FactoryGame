// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FactoryMenuWidget.h"
#include "PauseMenuWidget.generated.h"

/**
 *  In-game pause menu with Resume, Save and Return to Main Menu
 */
UCLASS()
class FACTORYGAME_API UPauseMenuWidget : public UFactoryMenuWidget
{
	GENERATED_BODY()

public:

	/** Called when the player wants to close the pause menu */
	FSimpleDelegate OnResumeRequested;

protected:

	/** Builds the menu layout */
	virtual UWidget* BuildContent() override;

	/** Resumes the game */
	virtual bool HandleEscape() override;

private:

	/** Status line below the buttons */
	FText StatusMessage;
	bool bStatusIsError = false;
};
