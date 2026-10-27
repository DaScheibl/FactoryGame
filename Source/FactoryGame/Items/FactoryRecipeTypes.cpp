// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryRecipeTypes.h"

namespace
{
	// leere Spalten (keine ID oder Menge 0) gehören nicht zum Rezept
	void AddColumn(TArray<FFactoryItemStack>& Stacks, FName ItemId, int32 Amount)
	{
		if (!ItemId.IsNone() && Amount > 0)
		{
			Stacks.Emplace(ItemId, Amount);
		}
	}
}

TArray<FFactoryItemStack> FFactoryRecipeDefinition::GetInputs() const
{
	TArray<FFactoryItemStack> Inputs;
	AddColumn(Inputs, Input1, Input1Amount);
	AddColumn(Inputs, Input2, Input2Amount);
	AddColumn(Inputs, Input3, Input3Amount);
	AddColumn(Inputs, Input4, Input4Amount);
	return Inputs;
}

TArray<FFactoryItemStack> FFactoryRecipeDefinition::GetOutputs() const
{
	TArray<FFactoryItemStack> Outputs;
	AddColumn(Outputs, Output1, Output1Amount);
	AddColumn(Outputs, Output2, Output2Amount);
	return Outputs;
}

float FFactoryRecipeDefinition::GetPerMinute(int32 AmountPerCycle) const
{
	return Duration > 0.0f ? AmountPerCycle * 60.0f / Duration : 0.0f;
}
