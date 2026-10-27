// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 *  Gemeinsamer Header des Spielmoduls.
 *  LogFactoryGame ist die Log-Kategorie für alle eigenen Meldungen, z. B.
 *  UE_LOG(LogFactoryGame, Warning, TEXT("...")). Im Output Log nach "LogFactoryGame" filtern.
 */
/** Main log category used across the project */
DECLARE_LOG_CATEGORY_EXTERN(LogFactoryGame, Log, All);