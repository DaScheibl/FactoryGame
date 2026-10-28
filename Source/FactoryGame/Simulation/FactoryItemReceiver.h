// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FactoryItemReceiver.generated.h"

UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UFactoryItemReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 *  Anything a conveyor belt can hand items to: machine inputs, other belts, later splitters and storage.
 *
 *  Das Band ruft ReceiveItem auf, sobald ein Item sein Ende erreicht. Nimmt der Empfänger es nicht an,
 *  bleibt das Item am Bandende stehen und das Band staut sich. Aufrufe kommen nur aus SimTick,
 *  Empfänger müssen also genauso deterministisch bleiben (siehe IFactorySimulated).
 */
class FACTORYGAME_API IFactoryItemReceiver
{
	GENERATED_BODY()

public:

	/**
	 *  Takes one item. Returns false if there is no room, the item then stays where it is.
	 *  Offset: how far the item has already moved past the hand-over point in this tick, in mm. Only belts use it
	 */
	virtual bool ReceiveItem(FName ItemId, int32 Offset) = 0;
};
