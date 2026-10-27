// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FactoryMenuWidget.h"
#include "FactoryHUDWidget.generated.h"

/** Look of a notification */
enum class EFactoryNotificationType : uint8
{
	Info,
	Positive,
	Warning
};

/**
 *  In-game HUD: crosshair, build hotbar, hint line and notifications.
 *  Never takes input, so it stays visible below the inventory and the pause menu.
 *
 *  Wie alle Menüs wird das HUD komplett in C++ gebaut (UFactoryMenuWidget) und bei jeder
 *  Änderung (neue Meldung, Hotbar-Auswahl, abgelaufener Hinweis) per RebuildMenu() neu erzeugt.
 *  Erzeugt wird es vom AFactoryGamePlayerController in BeginPlay.
 */
UCLASS()
class FACTORYGAME_API UFactoryHUDWidget : public UFactoryMenuWidget
{
	GENERATED_BODY()

public:

	/** Shows a notification in the top right corner */
	void ShowNotification(const FText& Text, EFactoryNotificationType Type = EFactoryNotificationType::Info);

	/** Shows "+Count Item" (or "-Count Item"), merging repeated notifications for the same item */
	void ShowItemNotification(FName ItemId, int32 Delta);

	/** Shows a hint above the hotbar for the given time */
	void ShowHint(const FText& Text, float Duration = 4.0f);

protected:

	/** Makes the HUD ignore all input */
	virtual void NativeOnInitialized() override;

	/** Removes expired notifications and hints */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Builds the HUD layout */
	virtual UWidget* BuildContent() override;

private:

	UWidget* BuildCrosshair();
	UWidget* BuildHotbar();
	UWidget* BuildNotifications();

	struct FNotification
	{
		FText Text;
		EFactoryNotificationType Type = EFactoryNotificationType::Info;
		double ExpiresAt = 0.0;

		/** Item notifications with the same item and sign are merged */
		FName ItemId;
		int32 ItemDelta = 0;
	};

	TArray<FNotification> Notifications;

	FText HintText;
	double HintExpiresAt = 0.0;
};
