// Copyright Epic Games, Inc. All Rights Reserved.


#include "FactoryGameCameraManager.h"

AFactoryGameCameraManager::AFactoryGameCameraManager()
{
	// Der Camera Manager wird vom Player Controller erzeugt (PlayerCameraManagerClass).
	// Pitch-Grenzen in Grad: verhindert, dass man über Kopf nach hinten schauen kann.
	// set the min/max pitch
	ViewPitchMin = -70.0f;
	ViewPitchMax = 80.0f;
}
