// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "FactoryGameUserSettings.generated.h"

/**
 *  Project specific user settings.
 *  Extends the engine settings (resolution, window mode, scalability, vsync) with the UI language.
 *  Registered through GameUserSettingsClassName in DefaultEngine.ini.
 */
UCLASS(config=GameUserSettings, configdonotcheckdefaults)
class FACTORYGAME_API UFactoryGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:

	/** Returns the active settings object, or nullptr if the engine is configured with a different settings class */
	static UFactoryGameUserSettings* GetFactoryGameUserSettings();

	/** Returns the selected UI language code ("en" or "de") */
	FString GetLanguage() const;

	/** Sets the UI language code ("en" or "de") */
	void SetLanguage(const FString& InLanguage);

	/** Applies all settings, including the UI language */
	virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;

	/** Resets all settings to their defaults */
	virtual void SetToDefaults() override;

	/** Supported language codes */
	static const TArray<FString>& GetSupportedLanguages();

protected:

	/** Selected UI language code. Empty means "use the system language" */
	UPROPERTY(config)
	FString Language;
};
