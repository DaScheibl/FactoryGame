// Copyright Epic Games, Inc. All Rights Reserved.

#include "FactoryMenuWidget.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/CoreStyle.h"

const FLinearColor UFactoryMenuWidget::TextColor = FLinearColor(FColor(235, 236, 240));
const FLinearColor UFactoryMenuWidget::SubtleTextColor = FLinearColor(FColor(150, 155, 168));
const FLinearColor UFactoryMenuWidget::AccentColor = FLinearColor(FColor(240, 132, 36));
const FLinearColor UFactoryMenuWidget::ErrorColor = FLinearColor(FColor(235, 80, 70));

namespace
{
	const float CornerRadius = 6.0f;

	// Button-Aussehen: Normal = dunkel, Primary = orange (Hauptaktion), Selected = dunkel mit orangem Rand
	FButtonStyle MakeButtonStyle(EFactoryButtonType Type)
	{
		const FLinearColor Accent = UFactoryMenuWidget::AccentColor;
		const FLinearColor AccentHovered = FLinearColor(FColor(255, 160, 70));
		const FLinearColor AccentPressed = FLinearColor(FColor(190, 98, 20));
		const FLinearColor Dark = FLinearColor(FColor(28, 30, 36, 220));
		const FLinearColor DarkHovered = FLinearColor(FColor(62, 50, 40, 235));
		const FLinearColor Disabled = FLinearColor(FColor(24, 25, 30, 140));

		FButtonStyle Style;

		switch (Type)
		{
		case EFactoryButtonType::Primary:
			Style.SetNormal(FSlateRoundedBoxBrush(Accent, CornerRadius));
			Style.SetHovered(FSlateRoundedBoxBrush(AccentHovered, CornerRadius));
			break;

		case EFactoryButtonType::Selected:
			Style.SetNormal(FSlateRoundedBoxBrush(DarkHovered, CornerRadius, Accent, 1.5f));
			Style.SetHovered(FSlateRoundedBoxBrush(DarkHovered, CornerRadius, AccentHovered, 1.5f));
			break;

		default:
			Style.SetNormal(FSlateRoundedBoxBrush(Dark, CornerRadius));
			Style.SetHovered(FSlateRoundedBoxBrush(DarkHovered, CornerRadius, Accent, 1.0f));
			break;
		}

		Style.SetPressed(FSlateRoundedBoxBrush(AccentPressed, CornerRadius));
		Style.SetDisabled(FSlateRoundedBoxBrush(Disabled, CornerRadius));

		Style.SetNormalForeground(UFactoryMenuWidget::TextColor);
		Style.SetHoveredForeground(FLinearColor::White);
		Style.SetPressedForeground(FLinearColor::White);
		Style.SetDisabledForeground(FLinearColor(FColor(95, 98, 108)));

		Style.SetNormalPadding(FMargin(0.0f));
		Style.SetPressedPadding(FMargin(0.0f));

		return Style;
	}
}

void UFactoryMenuAction::Execute()
{
	if (Callback)
	{
		Callback();
	}
}

void UFactoryMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);

	// the root stays the same, only its content is replaced when rebuilding
	RootBorder = NewWidget<UBorder>();
	RootBorder->SetBrushColor(FLinearColor::Transparent);
	RootBorder->SetPadding(FMargin(0.0f));
	WidgetTree->RootWidget = RootBorder;

	RebuildMenu();
}

FReply UFactoryMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape && HandleEscape())
	{
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UFactoryMenuWidget::RebuildMenu()
{
	if (!RootBorder)
	{
		return;
	}

	// the old actions stay alive until the next garbage collection, so it's safe to rebuild from inside a click handler
	Actions.Reset();

	RootBorder->SetContent(BuildContent());
}

UTextBlock* UFactoryMenuWidget::MakeText(const FText& Text, float FontSize, const FLinearColor& Color, bool bBold)
{
	UTextBlock* TextBlock = NewWidget<UTextBlock>();
	TextBlock->SetText(Text);
	TextBlock->SetFont(FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", FMath::RoundToInt(FontSize)));
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
	TextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));
	return TextBlock;
}

UButton* UFactoryMenuWidget::MakeButton(UWidget* Content, TFunction<void()> OnClicked, EFactoryButtonType Type, const FMargin& ContentPadding)
{
	UButton* Button = NewWidget<UButton>();
	Button->SetStyle(MakeButtonStyle(Type));

	if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(Button->AddChild(Content)))
	{
		ButtonSlot->SetPadding(ContentPadding);
		ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
		ButtonSlot->SetVerticalAlignment(VAlign_Center);
	}

	// Callback in ein UObject packen (siehe UFactoryMenuAction) und in Actions festhalten,
	// damit der Garbage Collector es nicht wegräumt, solange der Button existiert
	UFactoryMenuAction* Action = NewObject<UFactoryMenuAction>(this);
	Action->Callback = MoveTemp(OnClicked);
	Actions.Add(Action);

	Button->OnClicked.AddDynamic(Action, &UFactoryMenuAction::Execute);

	return Button;
}

UButton* UFactoryMenuWidget::MakeTextButton(const FText& Label, TFunction<void()> OnClicked, EFactoryButtonType Type, float FontSize, ETextJustify::Type Justification)
{
	UTextBlock* LabelText = MakeText(Label, FontSize, TextColor, Type == EFactoryButtonType::Primary);

	// follow the button's foreground color so disabled buttons are dimmed
	LabelText->SetColorAndOpacity(FSlateColor::UseForeground());
	LabelText->SetJustification(Justification);

	return MakeButton(LabelText, MoveTemp(OnClicked), Type);
}

UBorder* UFactoryMenuWidget::MakePanel(UWidget* Content)
{
	UBorder* Panel = NewWidget<UBorder>();
	Panel->SetBrush(FSlateRoundedBoxBrush(FLinearColor(FColor(14, 15, 19, 225)), 10.0f, FLinearColor(FColor(255, 255, 255, 20)), 1.0f));
	Panel->SetPadding(FMargin(32.0f, 28.0f));
	Panel->SetContent(Content);
	return Panel;
}

UWidget* UFactoryMenuWidget::MakeColorBlock(const FLinearColor& Color, float Width, float Height)
{
	UImage* Image = NewWidget<UImage>();
	Image->SetColorAndOpacity(Color);
	return (Width > 0.0f || Height > 0.0f) ? MakeSized(Image, Width, Height) : Image;
}

UWidget* UFactoryMenuWidget::MakeSpacer(float Height)
{
	USpacer* Spacer = NewWidget<USpacer>();
	Spacer->SetSize(FVector2D(1.0f, Height));
	return Spacer;
}

UWidget* UFactoryMenuWidget::MakeSized(UWidget* Content, float Width, float Height)
{
	USizeBox* SizeBox = NewWidget<USizeBox>();

	if (Width > 0.0f)
	{
		SizeBox->SetWidthOverride(Width);
	}

	if (Height > 0.0f)
	{
		SizeBox->SetHeightOverride(Height);
	}

	SizeBox->SetContent(Content);
	return SizeBox;
}

UWidget* UFactoryMenuWidget::MakeCycleRow(const FText& Label, const FText& Value, TFunction<void(int32)> OnStep)
{
	UHorizontalBox* Row = NewWidget<UHorizontalBox>();

	AddColumn(Row, MakeText(Label, 18.0f), true);

	// beide Pfeil-Buttons teilen sich denselben Callback, deshalb als Shared Pointer
	const TSharedRef<TFunction<void(int32)>> Step = MakeShared<TFunction<void(int32)>>(MoveTemp(OnStep));

	AddColumn(Row, MakeSized(MakeTextButton(FText::FromString(TEXT("<")), [Step]() { (*Step)(-1); }, EFactoryButtonType::Normal, 18.0f, ETextJustify::Center), 44.0f, 40.0f));

	UTextBlock* ValueText = MakeText(Value, 18.0f, AccentColor, true);
	ValueText->SetJustification(ETextJustify::Center);
	AddColumn(Row, MakeSized(ValueText, 190.0f));

	AddColumn(Row, MakeSized(MakeTextButton(FText::FromString(TEXT(">")), [Step]() { (*Step)(1); }, EFactoryButtonType::Normal, 18.0f, ETextJustify::Center), 44.0f, 40.0f), false, FMargin(0.0f));

	return Row;
}

void UFactoryMenuWidget::AddRow(UVerticalBox* Box, UWidget* Child, const FMargin& RowPadding)
{
	if (UVerticalBoxSlot* BoxSlot = Box->AddChildToVerticalBox(Child))
	{
		BoxSlot->SetPadding(RowPadding);
		BoxSlot->SetHorizontalAlignment(HAlign_Fill);
	}
}

void UFactoryMenuWidget::AddColumn(UHorizontalBox* Box, UWidget* Child, bool bFill, const FMargin& ColumnPadding)
{
	if (UHorizontalBoxSlot* BoxSlot = Box->AddChildToHorizontalBox(Child))
	{
		BoxSlot->SetPadding(ColumnPadding);
		BoxSlot->SetVerticalAlignment(VAlign_Center);
		BoxSlot->SetSize(bFill ? FSlateChildSize(ESlateSizeRule::Fill) : FSlateChildSize(ESlateSizeRule::Automatic));
	}
}
