// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 *  Minimal UI string table for English and German.
 *  The language is taken from UFactoryGameUserSettings.
 *
 *  Bewusst einfach gehalten: alle UI-Texte stehen als Tabelle (Schlüssel → Englisch/Deutsch)
 *  in FactoryLocalization.cpp. Neue Texte dort eintragen und mit FactoryLoc::Get("Schlüssel")
 *  abrufen. Fehlende Schlüssel erscheinen im Spiel als <Schlüssel> und fallen so sofort auf.
 *  Die Sprache wird bei jedem Aufruf neu abgefragt; nach einem Sprachwechsel reicht daher
 *  ein RebuildMenu() der offenen Widgets.
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
