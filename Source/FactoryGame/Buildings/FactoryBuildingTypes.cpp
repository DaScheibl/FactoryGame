// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryBuildingTypes.h"
#include "FactoryLocalization.h"

namespace
{
	// unbenutzte Spalten (keine ID oder Menge 0) werden übersprungen
	void AddCostColumn(TArray<FFactoryItemStack>& Stacks, FName ItemId, int32 Amount)
	{
		if (!ItemId.IsNone() && Amount > 0)
		{
			Stacks.Emplace(ItemId, Amount);
		}
	}

	// Ports nebeneinander im Abstand von 2 m, mittig auf der Kante; passt das nicht, im Abstand von 1 m
	void AddPorts(TArray<FFactoryPort>& Ports, int32 Count, double EdgeX, double Width, bool bInput)
	{
		const double Spacing = (Count - 1) * 200.0 <= Width - 100.0 ? 200.0 : 100.0;

		for (int32 Index = 0; Index < Count; ++Index)
		{
			FFactoryPort& Port = Ports.AddDefaulted_GetRef();
			Port.Location = FVector(EdgeX, (Index - (Count - 1) * 0.5) * Spacing, 100.0);
			Port.Direction = FVector::ForwardVector;
			Port.bInput = bInput;
		}
	}
}

TArray<FFactoryItemStack> FFactoryBuildingDefinition::GetCosts() const
{
	TArray<FFactoryItemStack> Costs;
	AddCostColumn(Costs, Cost1, Cost1Amount);
	AddCostColumn(Costs, Cost2, Cost2Amount);
	AddCostColumn(Costs, Cost3, Cost3Amount);
	AddCostColumn(Costs, Cost4, Cost4Amount);
	return Costs;
}

TArray<FFactoryPort> FFactoryBuildingDefinition::GetPorts() const
{
	// Items fließen von hinten (-X) nach vorn (+X): Eingänge hinten, Ausgänge vorn
	TArray<FFactoryPort> Ports;
	AddPorts(Ports, Inputs, SizeX * -50.0, SizeY * 100.0, true);
	AddPorts(Ports, Outputs, SizeX * 50.0, SizeY * 100.0, false);
	return Ports;
}

FText FFactoryBuildingDefinition::GetDisplayName() const
{
	// fehlt die deutsche Übersetzung, wird der englische Name angezeigt
	return FText::FromString(FactoryLoc::IsGerman() && !NameDE.IsEmpty() ? NameDE : NameEN);
}
