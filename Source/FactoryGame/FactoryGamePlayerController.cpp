// Copyright Epic Games, Inc. All Rights Reserved.


#include "FactoryGamePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "InputMappingContext.h"
#include "FactoryGameCameraManager.h"
#include "FactoryGameInstance.h"
#include "PauseMenuWidget.h"
#include "FactoryHUDWidget.h"
#include "FactoryInventoryWidget.h"
#include "FactoryInventoryComponent.h"
#include "FactoryGameCharacter.h"
#include "FactoryItemDatabase.h"
#include "FactoryCheatManager.h"
#include "FactoryLocalization.h"
#include "Blueprint/UserWidget.h"
#include "FactoryGame.h"
#include "Widgets/Input/SVirtualJoystick.h"

AFactoryGamePlayerController::AFactoryGamePlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AFactoryGameCameraManager::StaticClass();

	// set the default menus
	PauseMenuWidgetClass = UPauseMenuWidget::StaticClass();
	HUDWidgetClass = UFactoryHUDWidget::StaticClass();
	InventoryWidgetClass = UFactoryInventoryWidget::StaticClass();

	// console commands for testing
	CheatClass = UFactoryCheatManager::StaticClass();

	HotbarSlots.SetNum(NumHotbarSlots);
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

	if (IsLocalPlayerController() && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UFactoryHUDWidget>(this, HUDWidgetClass);

		if (HUDWidget)
		{
			HUDWidget->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(LogFactoryGame, Error, TEXT("Could not spawn HUD widget."));
		}
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

	// inventory
	InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AFactoryGamePlayerController::ToggleInventory);
	InputComponent->BindKey(EKeys::I, IE_Pressed, this, &AFactoryGamePlayerController::ToggleInventory);
	InputComponent->BindKey(EKeys::Gamepad_Special_Left, IE_Pressed, this, &AFactoryGamePlayerController::ToggleInventory);

	// hotbar: number keys and d-pad
	for (const FKey& Key : { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six, EKeys::Seven, EKeys::Eight, EKeys::Nine, EKeys::Zero })
	{
		InputComponent->BindKey(Key, IE_Pressed, this, &AFactoryGamePlayerController::HandleHotbarKey);
	}

	InputComponent->BindKey(EKeys::Gamepad_DPad_Left, IE_Pressed, this, &AFactoryGamePlayerController::SelectPreviousHotbarSlot);
	InputComponent->BindKey(EKeys::Gamepad_DPad_Right, IE_Pressed, this, &AFactoryGamePlayerController::SelectNextHotbarSlot);

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

	// the pause menu replaces the inventory
	if (InventoryWidget && InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromParent();
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

	RestoreGameInput();
}

void AFactoryGamePlayerController::RestoreGameInput()
{
	const bool bPauseOpen = PauseMenuWidget && PauseMenuWidget->IsInViewport();
	const bool bInventoryOpen = InventoryWidget && InventoryWidget->IsInViewport();

	if (bPauseOpen || bInventoryOpen)
	{
		return;
	}

	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
}

void AFactoryGamePlayerController::ToggleInventory()
{
	if (InventoryWidget && InventoryWidget->IsInViewport())
	{
		CloseInventory();
	}
	else
	{
		OpenInventory();
	}
}

void AFactoryGamePlayerController::OpenInventory()
{
	if (!IsLocalPlayerController() || !InventoryWidgetClass || (PauseMenuWidget && PauseMenuWidget->IsInViewport()))
	{
		return;
	}

	const AFactoryGameCharacter* FactoryCharacter = Cast<AFactoryGameCharacter>(GetPawn());

	if (!FactoryCharacter)
	{
		return;
	}

	if (!InventoryWidget)
	{
		InventoryWidget = CreateWidget<UFactoryInventoryWidget>(this, InventoryWidgetClass);

		if (!InventoryWidget)
		{
			UE_LOG(LogFactoryGame, Error, TEXT("Could not spawn inventory widget."));
			return;
		}

		InventoryWidget->OnCloseRequested.BindUObject(this, &AFactoryGamePlayerController::CloseInventory);
	}

	InventoryWidget->SetInventory(FactoryCharacter->GetInventory(), FactoryCharacter->GetEquipment());
	InventoryWidget->AddToPlayerScreen(50);

	// the game keeps running while the inventory is open
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
	SetInputMode(InputMode);
	SetShowMouseCursor(true);

	// the input mode focuses the whole widget, move the focus onto a slot for gamepad navigation
	InventoryWidget->FocusCurrentSlot();
}

void AFactoryGamePlayerController::CloseInventory()
{
	if (InventoryWidget)
	{
		InventoryWidget->RemoveFromParent();
	}

	RestoreGameInput();
}

int32 AFactoryGamePlayerController::GiveItems(FName ItemId, int32 Count)
{
	const AFactoryGameCharacter* FactoryCharacter = Cast<AFactoryGameCharacter>(GetPawn());
	UFactoryInventoryComponent* Inventory = FactoryCharacter ? FactoryCharacter->GetInventory() : nullptr;

	if (!Inventory || Count <= 0)
	{
		return 0;
	}

	const int32 Added = Inventory->AddItem(ItemId, Count);

	if (HUDWidget)
	{
		if (Added > 0)
		{
			HUDWidget->ShowItemNotification(ItemId, Added);
		}

		if (Added < Count)
		{
			HUDWidget->ShowNotification(FactoryLoc::Get("Notify_InventoryFull"), EFactoryNotificationType::Warning);
		}
	}

	return Added;
}

void AFactoryGamePlayerController::SetHotbarSlot(int32 Index, FName Shortcut)
{
	if (!HotbarSlots.IsValidIndex(Index))
	{
		return;
	}

	HotbarSlots[Index] = Shortcut;

	if (HUDWidget)
	{
		HUDWidget->RebuildMenu();
	}
}

void AFactoryGamePlayerController::SetHotbarSlots(const TArray<FName>& InSlots)
{
	HotbarSlots = InSlots;
	HotbarSlots.SetNum(NumHotbarSlots);
	SelectedHotbarSlot = INDEX_NONE;

	if (HUDWidget)
	{
		HUDWidget->RebuildMenu();
	}
}

void AFactoryGamePlayerController::SelectHotbarSlot(int32 Index)
{
	if (!HotbarSlots.IsValidIndex(Index))
	{
		return;
	}

	SelectedHotbarSlot = SelectedHotbarSlot == Index ? INDEX_NONE : Index;

	if (!HUDWidget)
	{
		return;
	}

	if (SelectedHotbarSlot != INDEX_NONE && HotbarSlots[SelectedHotbarSlot].IsNone())
	{
		HUDWidget->ShowHint(FactoryLoc::Format("Hotbar_Empty", { FText::AsNumber((SelectedHotbarSlot + 1) % 10) }));
	}

	HUDWidget->RebuildMenu();
}

void AFactoryGamePlayerController::HandleHotbarKey(FKey Key)
{
	static const FKey NumberKeys[] = { EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four, EKeys::Five, EKeys::Six, EKeys::Seven, EKeys::Eight, EKeys::Nine, EKeys::Zero };

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(NumberKeys); ++Index)
	{
		if (NumberKeys[Index] == Key)
		{
			SelectHotbarSlot(Index);
			return;
		}
	}
}

void AFactoryGamePlayerController::SelectPreviousHotbarSlot()
{
	SelectHotbarSlot(SelectedHotbarSlot == INDEX_NONE ? NumHotbarSlots - 1 : (SelectedHotbarSlot + NumHotbarSlots - 1) % NumHotbarSlots);
}

void AFactoryGamePlayerController::SelectNextHotbarSlot()
{
	SelectHotbarSlot(SelectedHotbarSlot == INDEX_NONE ? 0 : (SelectedHotbarSlot + 1) % NumHotbarSlots);
}
