// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactorySimulated.h"
#include "FactoryItemReceiver.h"
#include "FactoryItemTypes.h"
#include "FactoryProductionComponent.generated.h"

/** What a production machine is currently doing */
UENUM(BlueprintType)
enum class EFactoryMachineState : uint8
{
	/** No recipe selected */
	NoRecipe,

	/** Not enough items in the input buffers to start a cycle */
	WaitingForInput,

	/** A cycle is running */
	Working,

	/** The output buffer has no room for the result */
	OutputFull,

	/** Could work, but its power network has no power (no generator, not enough power or the fuse tripped) */
	NoPower,

	/** Generator with fuel, but nothing in its network uses power */
	Standby
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFactoryMachineStateChanged, EFactoryMachineState);

/** Everything a production machine needs to continue after loading */
USTRUCT()
struct FACTORYGAME_API FFactoryProductionSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FName RecipeId;

	UPROPERTY(SaveGame)
	TArray<FFactoryItemStack> Inputs;

	UPROPERTY(SaveGame)
	TArray<FFactoryItemStack> Outputs;

	UPROPERTY(SaveGame)
	int32 Progress = 0;

	UPROPERTY(SaveGame)
	bool bCycleRunning = false;

	UPROPERTY(SaveGame)
	int64 CyclesCompleted = 0;

	UPROPERTY(SaveGame)
	int32 SpeedPercent = 100;

	/** Generators: the fuse of the network tripped and has to be switched on again */
	UPROPERTY(SaveGame)
	bool bFuseTripped = false;
};

/**
 *  Runs recipes in the factory simulation: takes items from the input buffers, waits the recipe duration and puts the result into the output buffers.
 *  Used by every production building (smelter, constructor, assembler ...).
 *
 *  Ablauf eines Durchlaufs:
 *  1. Liegen genug Zutaten in den Eingangspuffern und passt das Ergebnis noch in die Ausgangspuffer,
 *     werden die Zutaten sofort entnommen und der Durchlauf startet.
 *  2. Pro Simulations-Tick steigt der Fortschritt um SpeedPercent (100 = normal, später Overclocking).
 *  3. Ist der Fortschritt voll, landet das Ergebnis im Ausgangspuffer und es geht von vorn los.
 *  Jeder Ein- und Ausgang fasst einen Stapel des Items (Docs/Standards/12-simulation.md).
 *  Gerechnet wird nur mit ganzen Zahlen, damit die Simulation auf jedem PC gleich läuft.
 *  Die Komponente tickt nicht selbst, sondern wird vom UFactorySimulationSubsystem aufgerufen.
 */
UCLASS(ClassGroup=(Factory), meta=(BlueprintSpawnableComponent))
class FACTORYGAME_API UFactoryProductionComponent : public UActorComponent, public IFactorySimulated, public IFactoryItemReceiver
{
	GENERATED_BODY()

public:

	UFactoryProductionComponent();

	/** Called whenever the machine state changes, e.g. to switch the status light */
	FOnFactoryMachineStateChanged OnStateChanged;

	// IFactorySimulated
	virtual void SimTick(int64 Tick) override;

	// IFactoryItemReceiver: belts deliver into the input buffers
	virtual bool ReceiveItem(FName ItemId, int32 Offset) override;

	/** Returns the building id, e.g. BLD_Smelter */
	FName GetBuildingId() const { return BuildingId; }

	/** Sets the building id. Only recipes of this building can be selected */
	void SetBuildingId(FName InBuildingId) { BuildingId = InBuildingId; }

	FName GetRecipeId() const { return RecipeId; }

	/**
	 *  Selects a recipe (None clears it). Fails if the recipe doesn't exist or belongs to another building.
	 *  Items in the buffers and the ingredients of a running cycle are added to OutReturnedItems, e.g. to give them back to the player.
	 */
	bool SetRecipe(FName NewRecipeId, TArray<FFactoryItemStack>& OutReturnedItems);

	EFactoryMachineState GetState() const { return State; }

	/** Returns the progress of the running cycle from 0 to 1 */
	float GetProgress() const;

	/** Like GetProgress, but estimated between two simulation ticks so displays move smoothly. Only for visuals */
	float GetDisplayProgress() const;

	/** Returns the input buffers, one per recipe input */
	const TArray<FFactoryItemStack>& GetInputs() const { return Inputs; }

	/** Returns the output buffers, one per recipe output */
	const TArray<FFactoryItemStack>& GetOutputs() const { return Outputs; }

	/** Returns how many of the given item the input buffers can still take */
	int32 GetInputSpace(FName ItemId) const;

	/** Puts items into the matching input buffer. Returns the number of items that were taken */
	int32 InsertInput(FName ItemId, int32 Count);

	/** Takes items out of the output buffers (ItemId None: any output). Returns the taken items */
	FFactoryItemStack ExtractOutput(FName ItemId, int32 MaxCount);

	/** Takes items back out of an input buffer, e.g. when the player empties it by hand. Returns the taken items */
	FFactoryItemStack ExtractInput(FName ItemId, int32 MaxCount);

	/** Speed in percent, 100 = recipe speed. Used by overclocking later */
	int32 GetSpeedPercent() const { return SpeedPercent; }

	void SetSpeedPercent(int32 NewSpeedPercent) { SpeedPercent = FMath::Clamp(NewSpeedPercent, 1, 250); }

	/** Sets power use (machines) or power output (generators) in MW, from the building table */
	void SetPower(int32 InPowerMW, bool bInGenerator) { PowerMW = InPowerMW; bGenerator = bInGenerator; }

	int32 GetPowerMW() const { return PowerMW; }

	bool IsGenerator() const { return bGenerator; }

	/** True for machines that only work with power */
	bool NeedsPower() const { return !bGenerator && PowerMW > 0; }

	/** Set by UFactoryPowerSubsystem before every simulation tick */
	void SetPowered(bool bInPowered) { bPowered = bInPowered; }

	bool IsPowered() const { return bPowered; }

	/** True if the machine would run a cycle this tick when it has power. Counts as power use of its network */
	bool WantsToRun() const;

	/** Generators: true if a cycle is running or there is fuel for the next one */
	bool CanGenerate() const;

	/** Generators: how much of the output the network uses, in per mille (0 = standby). Burns fuel at that rate */
	void SetGeneratorLoad(int32 InLoadPermille) { GeneratorLoadPermille = FMath::Clamp(InLoadPermille, 0, 1000); }

	int32 GetGeneratorLoad() const { return GeneratorLoadPermille; }

	/** Generators: the fuse of the network tripped. Set and cleared by UFactoryPowerSubsystem */
	bool IsFuseTripped() const { return bFuseTripped; }

	void SetFuseTripped(bool bTripped) { bFuseTripped = bTripped; }

	/** Returns the state for the world save */
	FFactoryProductionSaveData GetSaveData() const;

	/** Restores a saved state. The building id must be set before */
	void LoadSaveData(const FFactoryProductionSaveData& Data);

	/** Returns the number of finished cycles since the machine was built */
	int64 GetCyclesCompleted() const { return CyclesCompleted; }

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Building this machine belongs to, e.g. BLD_Smelter */
	UPROPERTY(EditAnywhere, SaveGame, Category="Production")
	FName BuildingId;

	/** Selected recipe, None if the machine is idle */
	UPROPERTY(EditAnywhere, SaveGame, Category="Production")
	FName RecipeId;

	/** Speed in percent, see SetSpeedPercent */
	UPROPERTY(EditAnywhere, SaveGame, Category="Production", meta=(ClampMin=1, ClampMax=250))
	int32 SpeedPercent = 100;

private:

	/** Tries to start a new cycle. Returns true if it started */
	bool TryStartCycle();

	/** Returns true if the input buffers hold the ingredients of one cycle */
	bool HasInputsForCycle() const;

	/** SimTick of a generator: burns fuel at the load the power network asks for */
	void SimTickGenerator();

	/** Puts the result of the finished cycle into the output buffers. Returns false if there is no room */
	bool TryFinishCycle();

	/** Returns true if the result of one cycle fits into the output buffers */
	bool HasOutputRoom() const;

	/** Sets up empty buffers for the selected recipe */
	void ResetBuffers();

	void SetState(EFactoryMachineState NewState);

	/** Returns the capacity of one buffer for the given item: one stack */
	int32 GetBufferCapacity(FName ItemId) const;

	UPROPERTY(VisibleInstanceOnly, SaveGame, Category="Production")
	TArray<FFactoryItemStack> Inputs;

	UPROPERTY(VisibleInstanceOnly, SaveGame, Category="Production")
	TArray<FFactoryItemStack> Outputs;

	/** Progress of the running cycle. A cycle is done at CycleLength (ticks x 100) */
	UPROPERTY(VisibleInstanceOnly, SaveGame, Category="Production")
	int32 Progress = 0;

	/** True while a cycle is running (its ingredients were already taken) */
	UPROPERTY(VisibleInstanceOnly, SaveGame, Category="Production")
	bool bCycleRunning = false;

	UPROPERTY(VisibleInstanceOnly, SaveGame, Category="Production")
	int64 CyclesCompleted = 0;

	/** Recipe duration in ticks x 100, cached when the recipe is set */
	int32 CycleLength = 0;

	/** Power use or output in MW */
	int32 PowerMW = 0;

	bool bGenerator = false;

	bool bPowered = false;

	int32 GeneratorLoadPermille = 0;

	bool bFuseTripped = false;

	EFactoryMachineState State = EFactoryMachineState::NoRecipe;
};
