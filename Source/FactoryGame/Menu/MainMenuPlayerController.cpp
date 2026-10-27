// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenuPlayerController.h"
#include "MainMenuWidget.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

AMainMenuPlayerController::AMainMenuPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;

	MenuWidgetClass = UMainMenuWidget::StaticClass();
}

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController())
	{
		return;
	}

	// place the background camera above the first player start
	FVector CameraLocation = FVector(0.0f, 0.0f, CameraHeight);
	float CameraYaw = 0.0f;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		CameraLocation = It->GetActorLocation() + FVector(0.0f, 0.0f, CameraHeight);
		CameraYaw = It->GetActorRotation().Yaw;
		break;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// eigene Kamera spawnen und als Sicht setzen, weil es keinen Pawn mit Kamera gibt
	BackdropCamera = GetWorld()->SpawnActor<ACameraActor>(CameraLocation, FRotator(CameraPitch, CameraYaw, 0.0f), SpawnParams);

	if (BackdropCamera)
	{
		SetViewTarget(BackdropCamera);
	}

	// show the menu
	MenuWidget = CreateWidget<UMainMenuWidget>(this, MenuWidgetClass);

	if (MenuWidget)
	{
		MenuWidget->AddToPlayerScreen(0);

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(MenuWidget->TakeWidget());
		// nur UI-Eingaben; die Maus darf das Fenster verlassen
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
}

void AMainMenuPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (BackdropCamera)
	{
		// langsam um die Hochachse drehen (CameraYawSpeed Grad pro Sekunde)
		BackdropCamera->AddActorWorldRotation(FRotator(0.0f, CameraYawSpeed * DeltaSeconds, 0.0f));
	}
}
