// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryGame.h"
#include "Modules/ModuleManager.h"

// Registriert dieses Modul als Haupt-Spielmodul. Eigene Start-/Shutdown-Logik gibt es nicht,
// deshalb reicht die Standard-Implementierung FDefaultGameModuleImpl.
IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, FactoryGame, "FactoryGame" );

DEFINE_LOG_CATEGORY(LogFactoryGame)