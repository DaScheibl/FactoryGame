// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FactoryItemTypes.generated.h"

class UTexture2D;
class UStaticMesh;

/** Physical form of an item */
UENUM(BlueprintType)
enum class EFactoryItemForm : uint8
{
	/** Can be carried, put on belts and stored in chests */
	Solid,

	/** Only moves through pipes and tanks, can't be put into the inventory */
	Fluid
};

/** Stack size classes, see Docs/Standards/10-items-inventar.md */
UENUM(BlueprintType)
enum class EFactoryStackClass : uint8
{
	/** Small parts like screws and wire: 500 */
	Small,

	/** Ores, ingots, plates: 100 */
	Standard,

	/** Components like rotors and circuits: 50 */
	Component,

	/** Large parts like computers and rocket segments: 20 */
	Large,

	/** Equipment like tools and jetpacks: 1 */
	Single
};

/** Equipment slot an item can be worn in */
UENUM(BlueprintType)
enum class EFactoryEquipSlot : uint8
{
	/** Not wearable */
	None,

	/** Helmet, gas mask, headlamp */
	Head,

	/** Suit upgrades */
	Body,

	/** Jetpack, glider */
	Back,

	/** Active tool like the hand drill or scanner */
	Hand
};

/**
 *  One row of the item table (Content/FactoryGame/Core/Data/DT_Items.csv).
 *  The row name is the item id, e.g. ITM_IronPlate.
 */
USTRUCT(BlueprintType)
struct FACTORYGAME_API FFactoryItemDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FString NameEN;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FString NameDE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FString DescriptionEN;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FString DescriptionDE;

	/** Tier in which the item is unlocked (0-8) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item", meta=(ClampMin=0, ClampMax=8))
	int32 Tier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	EFactoryItemForm Form = EFactoryItemForm::Solid;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	EFactoryStackClass StackClass = EFactoryStackClass::Standard;

	/** Equipment slot the item can be worn in, None for normal items */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	EFactoryEquipSlot EquipSlot = EFactoryEquipSlot::None;

	/** Shown in the UI as long as there is no icon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FColor PlaceholderColor = FColor(138, 141, 143);

	/** 256 px icon from the icon rig */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Mesh shown on conveyor belts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	TSoftObjectPtr<UStaticMesh> BeltMesh;

	/** Returns the maximum stack size, or 0 if the item can't be carried */
	int32 GetMaxStackSize() const;

	/** Returns the display name in the current UI language */
	FText GetDisplayName() const;

	/** Returns the description in the current UI language */
	FText GetDescription() const;

	/** Returns the stack size of a stack class */
	static int32 GetStackSize(EFactoryStackClass StackClass);
};

/**
 *  A number of items of the same type, e.g. one inventory slot
 */
USTRUCT(BlueprintType)
struct FACTORYGAME_API FFactoryItemStack
{
	GENERATED_BODY()

	FFactoryItemStack() = default;
	FFactoryItemStack(FName InItemId, int32 InCount) : ItemId(InItemId), Count(InCount) {}

	/** Row name in the item table. None for an empty stack */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category="Item")
	FName ItemId;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, Category="Item")
	int32 Count = 0;

	bool IsEmpty() const { return ItemId.IsNone() || Count <= 0; }

	void Clear() { ItemId = NAME_None; Count = 0; }
};
