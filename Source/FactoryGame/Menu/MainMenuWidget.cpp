// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenuWidget.h"
#include "FactoryGameInstance.h"
#include "FactoryGameUserSettings.h"
#include "FactoryLocalization.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/CoreStyle.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
	/** Window modes in the order they are cycled through */
	const EWindowMode::Type WindowModes[] = { EWindowMode::Fullscreen, EWindowMode::WindowedFullscreen, EWindowMode::Windowed };
	const TCHAR* WindowModeKeys[] = { TEXT("WindowMode_Fullscreen"), TEXT("WindowMode_Borderless"), TEXT("WindowMode_Windowed") };

	const int32 NumQualityLevels = 5;

	/** Formats a play time like "2 h 05 min" */
	FText FormatPlayTime(double Seconds)
	{
		const int32 TotalMinutes = FMath::FloorToInt32(Seconds / 60.0);
		const int32 Hours = TotalMinutes / 60;
		const int32 Minutes = TotalMinutes % 60;

		return FText::FromString(Hours > 0 ? FString::Printf(TEXT("%d h %02d min"), Hours, Minutes) : FString::Printf(TEXT("%d min"), Minutes));
	}

	/** Wraps an index around the given count */
	int32 WrapIndex(int32 Index, int32 Count)
	{
		return Count > 0 ? (Index % Count + Count) % Count : 0;
	}
}

UWidget* UMainMenuWidget::BuildContent()
{
	WorldNameBox = nullptr;

	UOverlay* Root = NewWidget<UOverlay>();

	// darken the level in the background, with a stronger band behind the buttons
	if (UOverlaySlot* TintSlot = Root->AddChildToOverlay(MakeColorBlock(FLinearColor(0.0f, 0.0f, 0.0f, 0.35f))))
	{
		TintSlot->SetHorizontalAlignment(HAlign_Fill);
		TintSlot->SetVerticalAlignment(VAlign_Fill);
	}

	if (UOverlaySlot* BandSlot = Root->AddChildToOverlay(MakeColorBlock(FLinearColor(FColor(8, 9, 12, 190)), 440.0f)))
	{
		BandSlot->SetHorizontalAlignment(HAlign_Left);
		BandSlot->SetVerticalAlignment(VAlign_Fill);
	}

	if (UOverlaySlot* NavSlot = Root->AddChildToOverlay(BuildNavigation()))
	{
		NavSlot->SetHorizontalAlignment(HAlign_Left);
		NavSlot->SetVerticalAlignment(VAlign_Center);
		NavSlot->SetPadding(FMargin(64.0f, 0.0f, 0.0f, 0.0f));
	}

	UWidget* Page = nullptr;

	switch (ActivePage)
	{
	case EMainMenuPage::NewWorld:	Page = BuildNewWorldPage(); break;
	case EMainMenuPage::LoadWorld:	Page = BuildLoadWorldPage(); break;
	case EMainMenuPage::Settings:	Page = BuildSettingsPage(); break;
	default: break;
	}

	if (Page)
	{
		if (UOverlaySlot* PageSlot = Root->AddChildToOverlay(MakeSized(MakePanel(Page), 600.0f)))
		{
			PageSlot->SetHorizontalAlignment(HAlign_Left);
			PageSlot->SetVerticalAlignment(VAlign_Center);
			PageSlot->SetPadding(FMargin(500.0f, 40.0f, 40.0f, 40.0f));
		}
	}

	return Root;
}

bool UMainMenuWidget::HandleEscape()
{
	if (ActivePage != EMainMenuPage::None)
	{
		TogglePage(ActivePage);
		return true;
	}

	return false;
}

UWidget* UMainMenuWidget::BuildNavigation()
{
	UFactoryGameInstance* GameInstance = UFactoryGameInstance::Get(this);

	UVerticalBox* Nav = NewWidget<UVerticalBox>();

	AddRow(Nav, MakeText(FactoryLoc::Get("Menu_Title"), 48.0f, TextColor, true), FMargin(0.0f));
	AddRow(Nav, MakeColorBlock(AccentColor, 90.0f, 4.0f), FMargin(2.0f, 6.0f, 0.0f, 44.0f));

	// continue, with the name of the last world below the label
	FFactoryWorldInfo LastWorld;
	const bool bCanContinue = GameInstance && GameInstance->GetLastPlayedWorld(LastWorld);

	UVerticalBox* ContinueContent = NewWidget<UVerticalBox>();
	UTextBlock* ContinueLabel = MakeText(FactoryLoc::Get("Menu_Continue"), 22.0f, TextColor, true);
	ContinueLabel->SetColorAndOpacity(FSlateColor::UseForeground());
	AddRow(ContinueContent, ContinueLabel, FMargin(0.0f));

	if (bCanContinue)
	{
		AddRow(ContinueContent, MakeText(FactoryLoc::Format("Menu_ContinueHint", { FText::FromString(LastWorld.DisplayName) }), 13.0f, FLinearColor(1.0f, 1.0f, 1.0f, 0.85f)), FMargin(0.0f, 2.0f, 0.0f, 0.0f));
	}

	UButton* ContinueButton = MakeButton(ContinueContent, [this]()
	{
		if (UFactoryGameInstance* GI = UFactoryGameInstance::Get(this))
		{
			if (!GI->ContinueLastWorld())
			{
				ActivePage = EMainMenuPage::LoadWorld;
				SetStatus(FactoryLoc::Get("Load_Failed"), true);
			}
		}
	}, bCanContinue ? EFactoryButtonType::Primary : EFactoryButtonType::Normal);

	ContinueButton->SetIsEnabled(bCanContinue);
	AddRow(Nav, MakeSized(ContinueButton, 300.0f), FMargin(0.0f, 0.0f, 0.0f, 14.0f));

	// page buttons
	auto AddPageButton = [&](FName LabelKey, EMainMenuPage Page)
	{
		const EFactoryButtonType Type = ActivePage == Page ? EFactoryButtonType::Selected : EFactoryButtonType::Normal;
		AddRow(Nav, MakeSized(MakeTextButton(FactoryLoc::Get(LabelKey), [this, Page]() { TogglePage(Page); }, Type), 300.0f), FMargin(0.0f, 5.0f));
	};

	AddPageButton("Menu_NewWorld", EMainMenuPage::NewWorld);
	AddPageButton("Menu_LoadWorld", EMainMenuPage::LoadWorld);
	AddPageButton("Menu_Settings", EMainMenuPage::Settings);

	AddRow(Nav, MakeSized(MakeTextButton(FactoryLoc::Get("Menu_Quit"), [this]()
	{
		UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
	}), 300.0f), FMargin(0.0f, 30.0f, 0.0f, 0.0f));

	return Nav;
}

UWidget* UMainMenuWidget::BuildNewWorldPage()
{
	UVerticalBox* Page = NewWidget<UVerticalBox>();

	AddRow(Page, MakeText(FactoryLoc::Get("NewWorld_Header"), 30.0f, TextColor, true), FMargin(0.0f, 0.0f, 0.0f, 24.0f));
	AddRow(Page, MakeText(FactoryLoc::Get("NewWorld_NameLabel"), 16.0f, SubtleTextColor));

	// suggest a name for the first visit of the page
	if (PendingWorldName.IsEmpty())
	{
		const int32 WorldCount = UFactoryGameInstance::Get(this) ? UFactoryGameInstance::Get(this)->GetSavedWorlds().Num() : 0;
		PendingWorldName = FactoryLoc::Format("NewWorld_DefaultName", { WorldCount + 1 }).ToString();
	}

	WorldNameBox = NewWidget<UEditableTextBox>();
	WorldNameBox->SetText(FText::FromString(PendingWorldName));

	FEditableTextBoxStyle BoxStyle = WorldNameBox->GetWidgetStyle();
	BoxStyle.SetBackgroundImageNormal(FSlateRoundedBoxBrush(FLinearColor(FColor(30, 32, 38)), 6.0f, FLinearColor(FColor(70, 74, 84)), 1.0f));
	BoxStyle.SetBackgroundImageHovered(FSlateRoundedBoxBrush(FLinearColor(FColor(34, 36, 43)), 6.0f, FLinearColor(FColor(120, 90, 60)), 1.0f));
	BoxStyle.SetBackgroundImageFocused(FSlateRoundedBoxBrush(FLinearColor(FColor(34, 36, 43)), 6.0f, AccentColor, 1.5f));
	BoxStyle.SetBackgroundColor(FLinearColor::White);
	BoxStyle.SetForegroundColor(TextColor);
	BoxStyle.SetFocusedForegroundColor(FLinearColor::White);
	BoxStyle.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 18));
	BoxStyle.SetPadding(FMargin(12.0f, 10.0f));
	WorldNameBox->SetWidgetStyle(BoxStyle);

	AddRow(Page, WorldNameBox, FMargin(0.0f, 6.0f, 0.0f, 8.0f));

	AddPageFooter(Page, {
		MakeTextButton(FactoryLoc::Get("NewWorld_Create"), [this]() { CreateWorld(); }, EFactoryButtonType::Primary, 18.0f, ETextJustify::Center),
		MakeTextButton(FactoryLoc::Get("Common_Back"), [this]() { TogglePage(EMainMenuPage::NewWorld); }, EFactoryButtonType::Normal, 18.0f, ETextJustify::Center)
	});

	return Page;
}

UWidget* UMainMenuWidget::BuildLoadWorldPage()
{
	UFactoryGameInstance* GameInstance = UFactoryGameInstance::Get(this);
	const TArray<FFactoryWorldInfo> Worlds = GameInstance ? GameInstance->GetSavedWorlds() : TArray<FFactoryWorldInfo>();

	// keep the selection valid
	if (!Worlds.ContainsByPredicate([this](const FFactoryWorldInfo& Info) { return Info.SlotName == SelectedWorldSlot; }))
	{
		SelectedWorldSlot = Worlds.Num() > 0 ? Worlds[0].SlotName : FString();
		bConfirmDelete = false;
	}

	UVerticalBox* Page = NewWidget<UVerticalBox>();

	AddRow(Page, MakeText(FactoryLoc::Get("Load_Header"), 30.0f, TextColor, true), FMargin(0.0f, 0.0f, 0.0f, 20.0f));

	if (Worlds.IsEmpty())
	{
		AddRow(Page, MakeText(FactoryLoc::Get("Load_Empty"), 17.0f, SubtleTextColor), FMargin(0.0f, 8.0f, 0.0f, 16.0f));
	}
	else
	{
		UScrollBox* List = NewWidget<UScrollBox>();

		for (const FFactoryWorldInfo& Info : Worlds)
		{
			UVerticalBox* RowContent = NewWidget<UVerticalBox>();
			AddRow(RowContent, MakeText(FText::FromString(Info.DisplayName), 19.0f, TextColor, true), FMargin(0.0f));
			AddRow(RowContent, MakeText(FactoryLoc::Format("Load_Details", { FText::AsDateTime(Info.LastPlayedAt, EDateTimeStyle::Medium, EDateTimeStyle::Short), FormatPlayTime(Info.PlayTimeSeconds) }), 13.0f, SubtleTextColor), FMargin(0.0f, 3.0f, 0.0f, 0.0f));

			const FString SlotName = Info.SlotName;
			const EFactoryButtonType Type = SlotName == SelectedWorldSlot ? EFactoryButtonType::Selected : EFactoryButtonType::Normal;

			UButton* RowButton = MakeButton(RowContent, [this, SlotName]()
			{
				// a second click on the selected world loads it
				if (SelectedWorldSlot == SlotName)
				{
					LoadSelectedWorld();
					return;
				}

				SelectedWorldSlot = SlotName;
				bConfirmDelete = false;
				StatusMessage = FText::GetEmpty();
				Refresh();
			}, Type, FMargin(16.0f, 10.0f));

			List->AddChild(RowButton);
			List->AddChild(MakeSpacer(6.0f));
		}

		AddRow(Page, MakeSized(List, 0.0f, FMath::Min(Worlds.Num(), 5) * 70.0f), FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}

	UButton* LoadButton = MakeTextButton(FactoryLoc::Get("Load_Load"), [this]() { LoadSelectedWorld(); }, EFactoryButtonType::Primary, 18.0f, ETextJustify::Center);
	LoadButton->SetIsEnabled(!SelectedWorldSlot.IsEmpty());

	UButton* DeleteButton = MakeTextButton(FactoryLoc::Get(bConfirmDelete ? "Load_ConfirmDelete" : "Load_Delete"), [this]() { DeleteSelectedWorld(); }, EFactoryButtonType::Normal, 18.0f, ETextJustify::Center);
	DeleteButton->SetIsEnabled(!SelectedWorldSlot.IsEmpty());

	AddPageFooter(Page, {
		LoadButton,
		DeleteButton,
		MakeTextButton(FactoryLoc::Get("Common_Back"), [this]() { TogglePage(EMainMenuPage::LoadWorld); }, EFactoryButtonType::Normal, 18.0f, ETextJustify::Center)
	});

	return Page;
}

UWidget* UMainMenuWidget::BuildSettingsPage()
{
	const TArray<FString>& Languages = UFactoryGameUserSettings::GetSupportedLanguages();

	UVerticalBox* Page = NewWidget<UVerticalBox>();

	AddRow(Page, MakeText(FactoryLoc::Get("Settings_Header"), 30.0f, TextColor, true), FMargin(0.0f, 0.0f, 0.0f, 20.0f));

	AddRow(Page, MakeCycleRow(FactoryLoc::Get("Settings_Language"), FactoryLoc::Get(*(TEXT("Language_") + Languages[PendingLanguage])), [this](int32 Step)
	{
		PendingLanguage = WrapIndex(PendingLanguage + Step, UFactoryGameUserSettings::GetSupportedLanguages().Num());
		Refresh();
	}));

	AddRow(Page, MakeCycleRow(FactoryLoc::Get("Settings_WindowMode"), FactoryLoc::Get(WindowModeKeys[PendingWindowMode]), [this](int32 Step)
	{
		PendingWindowMode = WrapIndex(PendingWindowMode + Step, UE_ARRAY_COUNT(WindowModes));
		Refresh();
	}));

	const FIntPoint Resolution = Resolutions.IsValidIndex(PendingResolution) ? Resolutions[PendingResolution] : FIntPoint::ZeroValue;
	AddRow(Page, MakeCycleRow(FactoryLoc::Get("Settings_Resolution"), FText::FromString(FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y)), [this](int32 Step)
	{
		PendingResolution = WrapIndex(PendingResolution + Step, Resolutions.Num());
		Refresh();
	}));

	AddRow(Page, MakeCycleRow(FactoryLoc::Get("Settings_Quality"), FactoryLoc::Get(*FString::Printf(TEXT("Quality_%d"), PendingQuality)), [this](int32 Step)
	{
		PendingQuality = WrapIndex(PendingQuality + Step, NumQualityLevels);
		Refresh();
	}));

	AddRow(Page, MakeCycleRow(FactoryLoc::Get("Settings_VSync"), FactoryLoc::Get(bPendingVSync ? "Common_On" : "Common_Off"), [this](int32 Step)
	{
		bPendingVSync = !bPendingVSync;
		Refresh();
	}));

	AddRow(Page, MakeSpacer(8.0f));

	AddPageFooter(Page, {
		MakeTextButton(FactoryLoc::Get("Settings_Apply"), [this]() { ApplySettings(); }, EFactoryButtonType::Primary, 18.0f, ETextJustify::Center),
		MakeTextButton(FactoryLoc::Get("Common_Back"), [this]() { TogglePage(EMainMenuPage::Settings); }, EFactoryButtonType::Normal, 18.0f, ETextJustify::Center)
	});

	return Page;
}

void UMainMenuWidget::AddPageFooter(UVerticalBox* Page, const TArray<UWidget*>& Buttons)
{
	if (!StatusMessage.IsEmpty())
	{
		AddRow(Page, MakeText(StatusMessage, 15.0f, bStatusIsError ? ErrorColor : AccentColor), FMargin(0.0f, 4.0f));
	}

	UHorizontalBox* ButtonRow = NewWidget<UHorizontalBox>();

	for (int32 Index = 0; Index < Buttons.Num(); ++Index)
	{
		const bool bLast = Index == Buttons.Num() - 1;
		AddColumn(ButtonRow, Buttons[Index], true, FMargin(0.0f, 0.0f, bLast ? 0.0f : 10.0f, 0.0f));
	}

	AddRow(Page, ButtonRow, FMargin(0.0f, 16.0f, 0.0f, 0.0f));
}

void UMainMenuWidget::TogglePage(EMainMenuPage Page)
{
	ActivePage = ActivePage == Page ? EMainMenuPage::None : Page;

	StatusMessage = FText::GetEmpty();
	bConfirmDelete = false;
	PendingWorldName.Empty();

	if (ActivePage == EMainMenuPage::Settings)
	{
		ReadSettings();
	}

	RebuildMenu();
}

void UMainMenuWidget::SetStatus(const FText& Message, bool bIsError)
{
	StatusMessage = Message;
	bStatusIsError = bIsError;
	Refresh();
}

void UMainMenuWidget::Refresh()
{
	if (WorldNameBox)
	{
		PendingWorldName = WorldNameBox->GetText().ToString();
	}

	RebuildMenu();
}

void UMainMenuWidget::ReadSettings()
{
	UFactoryGameUserSettings* Settings = UFactoryGameUserSettings::GetFactoryGameUserSettings();

	if (!Settings)
	{
		return;
	}

	PendingLanguage = FMath::Max(0, UFactoryGameUserSettings::GetSupportedLanguages().IndexOfByKey(Settings->GetLanguage()));

	PendingWindowMode = 0;
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(WindowModes); ++Index)
	{
		if (WindowModes[Index] == Settings->GetFullscreenMode())
		{
			PendingWindowMode = Index;
		}
	}

	// collect the resolutions, making sure the current one is part of the list
	Resolutions.Reset();
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);

	if (Resolutions.IsEmpty())
	{
		Resolutions = { FIntPoint(1280, 720), FIntPoint(1600, 900), FIntPoint(1920, 1080), FIntPoint(2560, 1440), FIntPoint(3840, 2160) };
	}

	const FIntPoint CurrentResolution = Settings->GetScreenResolution();
	Resolutions.AddUnique(CurrentResolution);
	Resolutions.Sort([](const FIntPoint& A, const FIntPoint& B) { return A.X != B.X ? A.X < B.X : A.Y < B.Y; });
	PendingResolution = Resolutions.IndexOfByKey(CurrentResolution);

	// custom scalability settings are shown as epic
	const int32 Quality = Settings->GetOverallScalabilityLevel();
	PendingQuality = Quality >= 0 ? FMath::Clamp(Quality, 0, NumQualityLevels - 1) : 3;

	bPendingVSync = Settings->IsVSyncEnabled();
}

void UMainMenuWidget::ApplySettings()
{
	UFactoryGameUserSettings* Settings = UFactoryGameUserSettings::GetFactoryGameUserSettings();

	if (!Settings)
	{
		return;
	}

	Settings->SetLanguage(UFactoryGameUserSettings::GetSupportedLanguages()[PendingLanguage]);
	Settings->SetFullscreenMode(WindowModes[PendingWindowMode]);

	if (Resolutions.IsValidIndex(PendingResolution))
	{
		Settings->SetScreenResolution(Resolutions[PendingResolution]);
	}

	Settings->SetOverallScalabilityLevel(PendingQuality);
	Settings->SetVSyncEnabled(bPendingVSync);

	// applies and saves the settings
	Settings->ApplySettings(false);

	// rebuild in the new language
	SetStatus(FactoryLoc::Get("Settings_Applied"), false);
}

void UMainMenuWidget::CreateWorld()
{
	const FString Name = WorldNameBox ? WorldNameBox->GetText().ToString().TrimStartAndEnd() : FString();

	if (Name.IsEmpty())
	{
		SetStatus(FactoryLoc::Get("NewWorld_NameEmpty"), true);
		return;
	}

	UFactoryGameInstance* GameInstance = UFactoryGameInstance::Get(this);

	if (!GameInstance || !GameInstance->CreateNewWorld(Name))
	{
		SetStatus(FactoryLoc::Get("NewWorld_Failed"), true);
	}
}

void UMainMenuWidget::LoadSelectedWorld()
{
	UFactoryGameInstance* GameInstance = UFactoryGameInstance::Get(this);

	if (SelectedWorldSlot.IsEmpty() || !GameInstance || !GameInstance->LoadWorld(SelectedWorldSlot))
	{
		SetStatus(FactoryLoc::Get("Load_Failed"), true);
	}
}

void UMainMenuWidget::DeleteSelectedWorld()
{
	// ask for confirmation with the first click
	if (!bConfirmDelete)
	{
		bConfirmDelete = true;
		Refresh();
		return;
	}

	if (UFactoryGameInstance* GameInstance = UFactoryGameInstance::Get(this))
	{
		GameInstance->DeleteWorld(SelectedWorldSlot);
	}

	SelectedWorldSlot.Empty();
	bConfirmDelete = false;
	Refresh();
}
