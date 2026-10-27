// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FactoryGamePlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UPauseMenuWidget;

/**
 *  Simple first person Player Controller
 *  Manages the input mapping context.
 *  Overrides the Player Camera Manager class.
 */
UCLASS(abstract, config="Game")
class FACTORYGAME_API AFactoryGamePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	AFactoryGamePlayerController();

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Pause menu widget to spawn */
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

	/** Pointer to the pause menu widget */
	UPROPERTY()
	TObjectPtr<UPauseMenuWidget> PauseMenuWidget;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Saves the world when the game is closed */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Restores the saved player position after a world has been loaded */
	virtual void OnPossess(APawn* InPawn) override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

public:

	/** Opens or closes the pause menu */
	void TogglePauseMenu();

	/** Pauses the game and shows the pause menu */
	void OpenPauseMenu();

	/** Hides the pause menu and resumes the game */
	void ClosePauseMenu();
};
