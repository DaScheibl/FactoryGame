// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FactoryGameGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class AFactoryGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFactoryGameGameMode();
};



