// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryItemTypes.h"
#include "FactoryLocalization.h"

int32 FFactoryItemDefinition::GetStackSize(EFactoryStackClass StackClass)
{
	switch (StackClass)
	{
	case EFactoryStackClass::Small:		return 500;
	case EFactoryStackClass::Component:	return 50;
	case EFactoryStackClass::Large:		return 20;
	case EFactoryStackClass::Single:	return 1;
	default:							return 100;
	}
}

int32 FFactoryItemDefinition::GetMaxStackSize() const
{
	// Stackgröße 0 bedeutet "nicht tragbar"; daran erkennen Inventare, dass Fluide nicht hineindürfen
	return Form == EFactoryItemForm::Fluid ? 0 : GetStackSize(StackClass);
}

FText FFactoryItemDefinition::GetDisplayName() const
{
	// fehlt die deutsche Übersetzung, wird der englische Name angezeigt
	return FText::FromString(FactoryLoc::IsGerman() && !NameDE.IsEmpty() ? NameDE : NameEN);
}

FText FFactoryItemDefinition::GetDescription() const
{
	return FText::FromString(FactoryLoc::IsGerman() && !DescriptionDE.IsEmpty() ? DescriptionDE : DescriptionEN);
}
