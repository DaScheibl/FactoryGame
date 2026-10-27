// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryProductionComponent.h"
#include "FactorySimulationSubsystem.h"
#include "FactoryRecipeDatabase.h"
#include "FactoryItemDatabase.h"
#include "FactoryGame.h"

UFactoryProductionComponent::UFactoryProductionComponent()
{
	// gerechnet wird im Simulations-Takt, nicht im normalen Tick
	PrimaryComponentTick.bCanEverTick = false;
}

void UFactoryProductionComponent::BeginPlay()
{
	Super::BeginPlay();

	// im Level platzierte Maschine mit vorausgewähltem Rezept: Puffer anlegen
	if (!RecipeId.IsNone())
	{
		TArray<FFactoryItemStack> Ignored;
		const FName InitialRecipe = RecipeId;
		RecipeId = NAME_None;
		SetRecipe(InitialRecipe, Ignored);
	}

	if (UFactorySimulationSubsystem* Simulation = UFactorySimulationSubsystem::Get(this))
	{
		Simulation->Register(this);
	}
}

void UFactoryProductionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFactorySimulationSubsystem* Simulation = UFactorySimulationSubsystem::Get(this))
	{
		Simulation->Unregister(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UFactoryProductionComponent::SimTick(int64 Tick)
{
	if (RecipeId.IsNone())
	{
		SetState(EFactoryMachineState::NoRecipe);
		return;
	}

	// Zutaten werden beim Start entnommen; fehlen sie, wartet die Maschine
	if (!bCycleRunning && !TryStartCycle())
	{
		SetState(HasOutputRoom() ? EFactoryMachineState::WaitingForInput : EFactoryMachineState::OutputFull);
		return;
	}

	if (Progress < CycleLength)
	{
		Progress += SpeedPercent;
	}

	if (Progress < CycleLength)
	{
		SetState(EFactoryMachineState::Working);
		return;
	}

	// Durchlauf fertig; passt das Ergebnis nicht, bleibt der Fortschritt voll stehen
	if (TryFinishCycle())
	{
		SetState(EFactoryMachineState::Working);
	}
	else
	{
		SetState(EFactoryMachineState::OutputFull);
	}
}

bool UFactoryProductionComponent::SetRecipe(FName NewRecipeId, TArray<FFactoryItemStack>& OutReturnedItems)
{
	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);
	const FFactoryRecipeDefinition* Recipe = Recipes ? Recipes->FindRecipe(NewRecipeId) : nullptr;

	if (!NewRecipeId.IsNone() && (!Recipe || Recipe->Building != BuildingId))
	{
		UE_LOG(LogFactoryGame, Warning, TEXT("%s: recipe '%s' can't be used in '%s'."), *GetNameSafe(GetOwner()), *NewRecipeId.ToString(), *BuildingId.ToString());
		return false;
	}

	// alles, was in der Maschine steckt, geht an den Aufrufer zurück (Puffer + Zutaten des laufenden Durchlaufs)
	for (const TArray<FFactoryItemStack>* Buffers : { &Inputs, &Outputs })
	{
		for (const FFactoryItemStack& Stack : *Buffers)
		{
			if (!Stack.IsEmpty())
			{
				OutReturnedItems.Add(Stack);
			}
		}
	}

	const FFactoryRecipeDefinition* OldRecipe = Recipes ? Recipes->FindRecipe(RecipeId) : nullptr;

	if (bCycleRunning && OldRecipe)
	{
		OutReturnedItems.Append(OldRecipe->GetInputs());
	}

	RecipeId = Recipe ? NewRecipeId : NAME_None;
	CycleLength = Recipe ? UFactorySimulationSubsystem::SecondsToTicks(Recipe->Duration) * 100 : 0;
	Progress = 0;
	bCycleRunning = false;
	ResetBuffers();

	SetState(Recipe ? EFactoryMachineState::WaitingForInput : EFactoryMachineState::NoRecipe);
	return true;
}

void UFactoryProductionComponent::ResetBuffers()
{
	Inputs.Reset();
	Outputs.Reset();

	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);

	if (const FFactoryRecipeDefinition* Recipe = Recipes ? Recipes->FindRecipe(RecipeId) : nullptr)
	{
		// ein leerer Puffer pro Zutat/Produkt, die ID steht fest, die Menge beginnt bei 0
		for (const FFactoryItemStack& Input : Recipe->GetInputs())
		{
			Inputs.Emplace(Input.ItemId, 0);
		}

		for (const FFactoryItemStack& Output : Recipe->GetOutputs())
		{
			Outputs.Emplace(Output.ItemId, 0);
		}
	}
}

bool UFactoryProductionComponent::TryStartCycle()
{
	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);
	const FFactoryRecipeDefinition* Recipe = Recipes ? Recipes->FindRecipe(RecipeId) : nullptr;

	if (!Recipe || !HasOutputRoom())
	{
		return false;
	}

	const TArray<FFactoryItemStack> Needed = Recipe->GetInputs();

	// Puffer und Rezept-Eingänge haben dieselbe Reihenfolge (siehe ResetBuffers)
	if (Needed.Num() != Inputs.Num())
	{
		return false;
	}

	for (int32 Index = 0; Index < Needed.Num(); ++Index)
	{
		if (Inputs[Index].Count < Needed[Index].Count)
		{
			return false;
		}
	}

	for (int32 Index = 0; Index < Needed.Num(); ++Index)
	{
		Inputs[Index].Count -= Needed[Index].Count;
	}

	bCycleRunning = true;
	Progress = 0;
	return true;
}

bool UFactoryProductionComponent::TryFinishCycle()
{
	if (!HasOutputRoom())
	{
		return false;
	}

	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);
	const FFactoryRecipeDefinition* Recipe = Recipes ? Recipes->FindRecipe(RecipeId) : nullptr;
	const TArray<FFactoryItemStack> Results = Recipe ? Recipe->GetOutputs() : TArray<FFactoryItemStack>();

	for (int32 Index = 0; Index < Results.Num() && Index < Outputs.Num(); ++Index)
	{
		Outputs[Index].Count += Results[Index].Count;
	}

	bCycleRunning = false;
	Progress = 0;
	++CyclesCompleted;
	return true;
}

bool UFactoryProductionComponent::HasOutputRoom() const
{
	const UFactoryRecipeDatabase* Recipes = UFactoryRecipeDatabase::Get(this);
	const FFactoryRecipeDefinition* Recipe = Recipes ? Recipes->FindRecipe(RecipeId) : nullptr;

	if (!Recipe)
	{
		return false;
	}

	const TArray<FFactoryItemStack> Results = Recipe->GetOutputs();

	for (int32 Index = 0; Index < Results.Num() && Index < Outputs.Num(); ++Index)
	{
		if (Outputs[Index].Count + Results[Index].Count > GetBufferCapacity(Outputs[Index].ItemId))
		{
			return false;
		}
	}

	return true;
}

int32 UFactoryProductionComponent::GetBufferCapacity(FName ItemId) const
{
	// ein Stapel; Fluide (Stapel 0) bekommen ihre Puffergröße erst mit SYS_Fluids
	const UFactoryItemDatabase* Items = UFactoryItemDatabase::Get(this);
	return Items ? Items->GetMaxStackSize(ItemId) : 0;
}

float UFactoryProductionComponent::GetProgress() const
{
	return CycleLength > 0 ? FMath::Clamp(float(Progress) / CycleLength, 0.0f, 1.0f) : 0.0f;
}

int32 UFactoryProductionComponent::GetInputSpace(FName ItemId) const
{
	for (const FFactoryItemStack& Input : Inputs)
	{
		if (Input.ItemId == ItemId)
		{
			return FMath::Max(0, GetBufferCapacity(ItemId) - Input.Count);
		}
	}

	return 0;
}

int32 UFactoryProductionComponent::InsertInput(FName ItemId, int32 Count)
{
	const int32 Inserted = FMath::Clamp(Count, 0, GetInputSpace(ItemId));

	if (Inserted > 0)
	{
		Inputs.FindByPredicate([ItemId](const FFactoryItemStack& Input) { return Input.ItemId == ItemId; })->Count += Inserted;
	}

	return Inserted;
}

FFactoryItemStack UFactoryProductionComponent::ExtractOutput(FName ItemId, int32 MaxCount)
{
	for (FFactoryItemStack& Output : Outputs)
	{
		if ((ItemId.IsNone() || Output.ItemId == ItemId) && Output.Count > 0)
		{
			const int32 Taken = FMath::Min(Output.Count, MaxCount);
			Output.Count -= Taken;
			return FFactoryItemStack(Output.ItemId, Taken);
		}
	}

	return FFactoryItemStack();
}

void UFactoryProductionComponent::SetState(EFactoryMachineState NewState)
{
	if (State != NewState)
	{
		State = NewState;
		OnStateChanged.Broadcast(State);
	}
}
