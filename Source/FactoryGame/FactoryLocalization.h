// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 *  Minimal UI string table for English and German.
 *  The language is taken from UFactoryGameUserSettings.
 */
namespace FactoryLoc
{
	/** Returns the text for the given key in the currently selected language */
	FACTORYGAME_API FText Get(FName Key);

	/** Returns the formatted text for the given key, replacing {0}, {1}, ... with the given arguments */
	FACTORYGAME_API FText Format(FName Key, FFormatOrderedArguments Args);

	/** Returns true if the UI language is German */
	FACTORYGAME_API bool IsGerman();
}
