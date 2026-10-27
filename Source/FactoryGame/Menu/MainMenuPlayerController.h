// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

class UMainMenuWidget;
class ACameraActor;

/**
 *  Player Controller of the main menu.
 *  Shows the menu widget and slowly pans a camera over the level in the background.
 */
UCLASS()
class FACTORYGAME_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AMainMenuPlayerController();

protected:

	/** Main menu widget to create. Can be swapped for a Blueprint subclass */
	UPROPERTY(EditAnywhere, Category="Main Menu")
	TSubclassOf<UMainMenuWidget> MenuWidgetClass;

	/** Height of the background camera above the player start */
	UPROPERTY(EditAnywhere, Category="Main Menu|Camera", meta=(Units="cm"))
	float CameraHeight = 250.0f;

	/** Downwards pitch of the background camera */
	UPROPERTY(EditAnywhere, Category="Main Menu|Camera", meta=(Units="deg"))
	float CameraPitch = -12.0f;

	/** Pan speed of the background camera */
	UPROPERTY(EditAnywhere, Category="Main Menu|Camera", meta=(Units="deg"))
	float CameraYawSpeed = 3.0f;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Pans the background camera */
	virtual void Tick(float DeltaSeconds) override;

private:

	UPROPERTY()
	TObjectPtr<UMainMenuWidget> MenuWidget;

	UPROPERTY()
	TObjectPtr<ACameraActor> BackdropCamera;
};
