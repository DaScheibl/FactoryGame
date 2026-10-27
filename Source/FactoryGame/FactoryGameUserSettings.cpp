// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryGameUserSettings.h"
#include "Engine/Engine.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"

UFactoryGameUserSettings* UFactoryGameUserSettings::GetFactoryGameUserSettings()
{
	return GEngine ? Cast<UFactoryGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

const TArray<FString>& UFactoryGameUserSettings::GetSupportedLanguages()
{
	static const TArray<FString> Languages = { TEXT("en"), TEXT("de") };
	return Languages;
}

FString UFactoryGameUserSettings::GetLanguage() const
{
	if (GetSupportedLanguages().Contains(Language))
	{
		return Language;
	}

	// no valid language stored yet, fall back to the system language
	const FString SystemLanguage = FInternationalization::Get().GetDefaultLanguage()->GetTwoLetterISOLanguageName();
	return SystemLanguage == TEXT("de") ? TEXT("de") : TEXT("en");
}

void UFactoryGameUserSettings::SetLanguage(const FString& InLanguage)
{
	// unbekannte Codes werden zu Englisch. Auf die Platte geschrieben wird erst in ApplySettings()
	Language = GetSupportedLanguages().Contains(InLanguage) ? InLanguage : TEXT("en");
}

void UFactoryGameUserSettings::ApplySettings(bool bCheckForCommandLineOverrides)
{
	Super::ApplySettings(bCheckForCommandLineOverrides);

	// switch the culture for number and date formatting.
	// Skipped in the editor so playing in PIE doesn't change the editor language.
	if (!GIsEditor)
	{
		FInternationalization::Get().SetCurrentCulture(GetLanguage());
	}
}

void UFactoryGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();

	Language.Empty();
}
