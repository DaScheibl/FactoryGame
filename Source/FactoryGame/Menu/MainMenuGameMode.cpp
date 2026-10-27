// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenuGameMode.h"
#include "MainMenuPlayerController.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	// keine Spielfigur im Menü; der Controller zeigt nur das Menü und die Hintergrundkamera
	DefaultPawnClass = nullptr;
	PlayerControllerClass = AMainMenuPlayerController::StaticClass();
}

bool AMainMenuGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	return false;
}
