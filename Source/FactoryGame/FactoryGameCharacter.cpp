// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryGameCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FactoryGame.h"
#include "FactoryInventoryComponent.h"

AFactoryGameCharacter::AFactoryGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	// FirstPerson-Primitive werden mit eigenem FOV und eigener Skalierung gerendert (siehe Kamera),
	// dadurch ragen die Arme nicht in Wände hinein.
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	// Die Kamera hängt am Kopf-Bone der Arme und folgt der Controller-Rotation (Maus/Stick).
	// FirstPersonFieldOfView/Scale gelten nur für die Arme, nicht für die Welt.
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// Create the player inventory
	Inventory = CreateDefaultSubobject<UFactoryInventoryComponent>(TEXT("Inventory"));

	Equipment = CreateDefaultSubobject<UFactoryInventoryComponent>(TEXT("Equipment"));
	// Ausrüstung = Inventar mit Slot-Filtern: ein Slot pro Eintrag, der nur passende Items annimmt.
	// Die Reihenfolge ist auch die Reihenfolge im Spielstand (EquipmentSlots), also nicht umsortieren.
	Equipment->SetSlotFilters({ EFactoryEquipSlot::Head, EFactoryEquipSlot::Body, EFactoryEquipSlot::Back, EFactoryEquipSlot::Hand });

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	// überschreibt die Größe vom Anfang des Konstruktors (Template-Überbleibsel):
	// 34 cm Radius, 96 cm halbe Höhe
	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	// starkes Abbremsen in der Luft und etwas Luftsteuerung für ein direktes Sprunggefühl
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AFactoryGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Wird beim Possess aufgerufen und bindet die Input Actions (im Blueprint zugewiesen) an Funktionen.
	// Started/Completed = Taste gedrückt/losgelassen, Triggered = jeden Frame, solange aktiv.
	// Menü-, Inventar- und Hotbar-Tasten bindet dagegen AFactoryGamePlayerController.
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AFactoryGameCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AFactoryGameCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFactoryGameCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFactoryGameCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AFactoryGameCharacter::LookInput);
	}
	else
	{
		UE_LOG(LogFactoryGame, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AFactoryGameCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void AFactoryGameCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AFactoryGameCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AFactoryGameCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AFactoryGameCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AFactoryGameCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}
