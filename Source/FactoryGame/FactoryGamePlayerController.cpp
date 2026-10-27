// Copyright Epic Games, Inc. All Rights Reserved.


#include "FactoryGamePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "FactoryGameCameraManager.h"
#include "FactoryGameInstance.h"
#include "PauseMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "FactoryGame.h"
#include "Widgets/Input/SVirtualJoystick.h"

AFactoryGamePlayerController::AFactoryGamePlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AFactoryGameCameraManager::StaticClass();

	// set the default pause menu
	PauseMenuWidgetClass = UPauseMenuWidget::StaticClass();
}

void AFactoryGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// the main menu leaves the viewport in UI mode, so switch back to game input
	if (IsLocalPlayerController())
	{
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
	}

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogFactoryGame, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AFactoryGamePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// save while the pawn still exists when the game is closed
	if (EndPlayReason == EEndPlayReason::Quit || EndPlayReason == EEndPlayReason::EndPlayInEditor)
	{
		if (UFactoryGameInstance* GameInstance = UFactoryGameInstance::Get(this))
		{
			GameInstance->SaveCurrentWorld();
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AFactoryGamePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (UFactoryGameInstance* GameInstance = UFactoryGameInstance::Get(this))
	{
		GameInstance->RestorePlayerState(this);
	}
}

void AFactoryGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// pause menu keys. P is also bound because Escape ends Play In Editor by default
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AFactoryGamePlayerController::TogglePauseMenu);
	InputComponent->BindKey(EKeys::P, IE_Pressed, this, &AFactoryGamePlayerController::TogglePauseMenu);
	InputComponent->BindKey(EKeys::Gamepad_Special_Right, IE_Pressed, this, &AFactoryGamePlayerController::TogglePauseMenu);

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
	
}

bool AFactoryGamePlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void AFactoryGamePlayerController::TogglePauseMenu()
{
	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		ClosePauseMenu();
	}
	else
	{
		OpenPauseMenu();
	}
}

void AFactoryGamePlayerController::OpenPauseMenu()
{
	if (!IsLocalPlayerController() || !PauseMenuWidgetClass)
	{
		return;
	}

	if (!PauseMenuWidget)
	{
		PauseMenuWidget = CreateWidget<UPauseMenuWidget>(this, PauseMenuWidgetClass);

		if (!PauseMenuWidget)
		{
			UE_LOG(LogFactoryGame, Error, TEXT("Could not spawn pause menu widget."));
			return;
		}

		PauseMenuWidget->OnResumeRequested.BindUObject(this, &AFactoryGamePlayerController::ClosePauseMenu);
	}

	// rebuild to clear old status messages and pick up language changes
	PauseMenuWidget->RebuildMenu();
	PauseMenuWidget->AddToPlayerScreen(100);

	SetPause(true);

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void AFactoryGamePlayerController::ClosePauseMenu()
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->RemoveFromParent();
	}

	SetPause(false);

	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
}
