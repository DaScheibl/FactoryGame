// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

/**
 *  Game Mode used while the main menu is shown.
 *  Doesn't spawn a pawn, the player controller shows the menu over the level.
 */
UCLASS()
class FACTORYGAME_API AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	AMainMenuGameMode();

	/** Prevents spawning a pawn in the menu */
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
};
