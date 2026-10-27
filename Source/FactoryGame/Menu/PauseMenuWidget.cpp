// Copyright Epic Games, Inc. All Rights Reserved.

#include "PauseMenuWidget.h"
#include "FactoryGameInstance.h"
#include "FactoryLocalization.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

UWidget* UPauseMenuWidget::BuildContent()
{
	UOverlay* Root = NewWidget<UOverlay>();

	if (UOverlaySlot* TintSlot = Root->AddChildToOverlay(MakeColorBlock(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f))))
	{
		TintSlot->SetHorizontalAlignment(HAlign_Fill);
		TintSlot->SetVerticalAlignment(VAlign_Fill);
	}

	UVerticalBox* Content = NewWidget<UVerticalBox>();

	UTextBlock* Header = MakeText(FactoryLoc::Get("Pause_Header"), 34.0f, TextColor, true);
	Header->SetJustification(ETextJustify::Center);
	AddRow(Content, Header, FMargin(0.0f));
	AddRow(Content, MakeColorBlock(AccentColor, 0.0f, 3.0f), FMargin(120.0f, 8.0f, 120.0f, 24.0f));

	AddRow(Content, MakeTextButton(FactoryLoc::Get("Pause_Resume"), [this]()
	{
		OnResumeRequested.ExecuteIfBound();
	}, EFactoryButtonType::Primary, 20.0f, ETextJustify::Center), FMargin(0.0f, 5.0f));

	AddRow(Content, MakeTextButton(FactoryLoc::Get("Pause_Save"), [this]()
	{
		UFactoryGameInstance* GameInstance = UFactoryGameInstance::Get(this);
		const bool bSaved = GameInstance && GameInstance->SaveCurrentWorld();

		StatusMessage = FactoryLoc::Get(bSaved ? "Pause_Saved" : "Pause_SaveFailed");
		bStatusIsError = !bSaved;
		RebuildMenu();
	}, EFactoryButtonType::Normal, 20.0f, ETextJustify::Center), FMargin(0.0f, 5.0f));

	AddRow(Content, MakeTextButton(FactoryLoc::Get("Pause_MainMenu"), [this]()
	{
		if (UFactoryGameInstance* GameInstance = UFactoryGameInstance::Get(this))
		{
			GameInstance->ReturnToMainMenu();
		}
	}, EFactoryButtonType::Normal, 20.0f, ETextJustify::Center), FMargin(0.0f, 5.0f));

	if (!StatusMessage.IsEmpty())
	{
		UTextBlock* Status = MakeText(StatusMessage, 15.0f, bStatusIsError ? ErrorColor : AccentColor);
		Status->SetJustification(ETextJustify::Center);
		AddRow(Content, Status, FMargin(0.0f, 12.0f, 0.0f, 0.0f));
	}

	if (UOverlaySlot* PanelSlot = Root->AddChildToOverlay(MakeSized(MakePanel(Content), 400.0f)))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
		PanelSlot->SetVerticalAlignment(VAlign_Center);
	}

	return Root;
}

bool UPauseMenuWidget::HandleEscape()
{
	OnResumeRequested.ExecuteIfBound();
	return true;
}
