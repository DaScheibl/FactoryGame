// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryItemTypes.h"
#include "FactoryCrate.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class AFactoryGamePlayerController;

/**
 *  Demolition crate: holds the items that didn't fit into the inventory when something was demolished.
 *
 *  Graybox: kleiner brauner Würfel (1 m) mit der Anzahl Items darüber. Die Kiste hat kein Slot-Limit,
 *  damit beim Abreißen nie etwas verloren geht. Mit E (Gamepad X) holt der Spieler so viel heraus,
 *  wie ins Inventar passt; ist die Kiste leer, verschwindet sie.
 *  Wird mit der Welt gespeichert (UFactoryWorldSaveGame::Crates).
 */
UCLASS()
class FACTORYGAME_API AFactoryCrate : public AActor
{
	GENERATED_BODY()

public:

	AFactoryCrate();

	/** Adds items, merging stacks of the same item */
	void AddItems(const TArray<FFactoryItemStack>& Stacks);

	const TArray<FFactoryItemStack>& GetItems() const { return Items; }

	/** Moves as much as fits into the player's inventory. Destroys the crate once it is empty */
	void TakeItems(AFactoryGamePlayerController* PlayerController);

	/** Spawns a crate on the ground at the given location */
	static AFactoryCrate* SpawnCrate(UWorld* World, const FVector& Location, const TArray<FFactoryItemStack>& Stacks);

	virtual void Tick(float DeltaSeconds) override;

private:

	void UpdateLabel();

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UTextRenderComponent> Label;

	UPROPERTY(VisibleInstanceOnly, Category="Crate")
	TArray<FFactoryItemStack> Items;
};
