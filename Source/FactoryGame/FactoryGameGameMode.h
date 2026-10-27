// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FactoryGameGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 *
 *  Abstrakte C++-Basis. Benutzt wird der Blueprint BP_FirstPersonGameMode
 *  (DefaultEngine.ini → GlobalDefaultGameMode), dort sind Pawn- und Controller-Klasse eingetragen.
 *  Solange keine Welt aktiv ist, ersetzt UFactoryGameInstance::OverrideGameModeClass ihn
 *  durch AMainMenuGameMode.
 */
UCLASS(abstract)
class AFactoryGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFactoryGameGameMode();
};



