// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Containers/Ticker.h"
#include "FactoryWorldSaveGame.h"
#include "FactoryGameInstance.generated.h"

class APlayerController;

/**
 *  Game Instance that manages the saved worlds.
 *  As long as no world is active, every map is started with the main menu game mode.
 *  Starting, loading or continuing a world opens the gameplay map with the regular game mode.
 */
UCLASS(config=Game)
class FACTORYGAME_API UFactoryGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UFactoryGameInstance();

	/** Returns the Factory Game Instance for the given world context */
	static UFactoryGameInstance* Get(const UObject* WorldContextObject);

	/** Initialization */
	virtual void Init() override;

	/** Cleanup */
	virtual void Shutdown() override;

	/** Swaps the game mode for the main menu game mode while no world is active */
	virtual TSubclassOf<AGameModeBase> OverrideGameModeClass(TSubclassOf<AGameModeBase> GameModeClass, const FString& MapName, const FString& Options, const FString& Portal) const override;

	/** Returns all saved worlds, most recently played first */
	TArray<FFactoryWorldInfo> GetSavedWorlds() const;

	/** Returns true and fills OutInfo if there is a world to continue */
	bool GetLastPlayedWorld(FFactoryWorldInfo& OutInfo) const;

	/** Creates a new world with the given name and opens it */
	bool CreateNewWorld(const FString& DisplayName);

	/** Loads the world stored in the given slot and opens it */
	bool LoadWorld(const FString& SlotName);

	/** Loads the last played world */
	bool ContinueLastWorld();

	/** Deletes the world stored in the given slot */
	void DeleteWorld(const FString& SlotName);

	/** Saves the currently active world. Returns false if no world is active or saving failed */
	bool SaveCurrentWorld();

	/** Saves the current world and returns to the main menu */
	void ReturnToMainMenu();

	/** Returns true while a world is being played */
	bool IsWorldActive() const { return !CurrentWorldSlot.IsEmpty(); }

	/** Moves the player's pawn to the saved location after a world has been loaded */
	void RestorePlayerState(APlayerController* PlayerController);

protected:

	/** Map that is opened when a world is started */
	UPROPERTY(EditDefaultsOnly, Config, Category="Worlds")
	TSoftObjectPtr<UWorld> GameplayMap;

	/** Map shown in the background of the main menu */
	UPROPERTY(EditDefaultsOnly, Config, Category="Worlds")
	TSoftObjectPtr<UWorld> MainMenuMap;

	/** Time between automatic saves in seconds. 0 disables autosaving */
	UPROPERTY(EditDefaultsOnly, Config, Category="Worlds", meta=(ClampMin=0, Units="s"))
	float AutosaveInterval = 120.0f;

private:

	/** Loads or creates the world index */
	void LoadWorldIndex();

	/** Writes the world index to disk */
	void SaveWorldIndex();

	/** Returns the index entry for the given slot */
	FFactoryWorldInfo* FindWorldInfo(const FString& SlotName);

	/** Makes the given world the active one and opens the gameplay map */
	void StartWorld(const FString& SlotName, UFactoryWorldSaveGame* SaveGame);

	/** Copies the current pawn and camera state into the active save game */
	void CapturePlayerState();

	/** Opens the given map */
	void OpenMap(const TSoftObjectPtr<UWorld>& Map);

	/** Ticker callback for autosaving */
	bool HandleAutosave(float DeltaTime);

	/** List of all saved worlds */
	UPROPERTY()
	TObjectPtr<UFactoryWorldIndexSave> WorldIndex;

	/** Data of the currently active world */
	UPROPERTY()
	TObjectPtr<UFactoryWorldSaveGame> CurrentWorldSave;

	/** Slot of the currently active world. Empty while in the main menu */
	FString CurrentWorldSlot;

	/** Set when a loaded world has a player position that still needs to be applied */
	bool bPendingPlayerRestore = false;

	/** Time stamp used to accumulate the play time */
	double PlayTimeCounterStart = 0.0;

	/** Handle of the autosave ticker */
	FTSTicker::FDelegateHandle AutosaveTickerHandle;
};
