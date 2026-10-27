// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Framework/Text/TextLayout.h"
#include "FactoryMenuWidget.generated.h"

class UBorder;
class UButton;
class UTextBlock;
class UVerticalBox;
class UHorizontalBox;

/**
 *  Wraps a native callback so it can be bound to a UMG button's dynamic OnClicked delegate
 *  Hintergrund: UButton::OnClicked akzeptiert nur UFUNCTIONs, keine Lambdas. Dieses kleine Objekt
 *  hält das Lambda und wird mit seiner Execute()-UFUNCTION an den Button gebunden.
 */
UCLASS()
class UFactoryMenuAction : public UObject
{
	GENERATED_BODY()

public:

	/** Function to run when the button is clicked */
	TFunction<void()> Callback;

	UFUNCTION()
	void Execute();
};

/** Visual variants of menu buttons */
enum class EFactoryButtonType : uint8
{
	Normal,
	Primary,
	Selected
};

/**
 *  Base class for the menus. The widget layout is built in C++, so no Widget Blueprint is needed.
 *  Subclasses implement BuildContent() and call RebuildMenu() whenever their state changes.
 *
 *  Prinzip: Der Zustand liegt in Member-Variablen, BuildContent() erzeugt daraus jedes Mal den
 *  kompletten Widget-Baum neu. Das ist einfach und für die kleinen Menüs schnell genug.
 *  Die Make...-Helfer sorgen für einheitliche Farben, Schriften und Buttons in allen Menüs.
 */
UCLASS(abstract)
class FACTORYGAME_API UFactoryMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Rebuilds the whole widget layout, e.g. after a state or language change */
	void RebuildMenu();

	static const FLinearColor TextColor;
	static const FLinearColor SubtleTextColor;
	static const FLinearColor AccentColor;
	static const FLinearColor ErrorColor;

protected:

	/** Creates the root widget and builds the initial layout */
	virtual void NativeOnInitialized() override;

	/** Forwards the escape key to HandleEscape */
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	/** Creates the layout of the menu */
	virtual UWidget* BuildContent() PURE_VIRTUAL(UFactoryMenuWidget::BuildContent, return nullptr;);

	/** Called when escape is pressed. Return true if the key was handled */
	virtual bool HandleEscape() { return false; }

	/** Constructs a widget inside this widget's tree */
	template<typename WidgetT>
	WidgetT* NewWidget()
	{
		return WidgetTree->ConstructWidget<WidgetT>(WidgetT::StaticClass());
	}

	/** Creates a text block */
	UTextBlock* MakeText(const FText& Text, float FontSize, const FLinearColor& Color = TextColor, bool bBold = false);

	/** Creates a button with arbitrary content */
	UButton* MakeButton(UWidget* Content, TFunction<void()> OnClicked, EFactoryButtonType Type = EFactoryButtonType::Normal, const FMargin& ContentPadding = FMargin(20.0f, 12.0f));

	/** Creates a button with a text label */
	UButton* MakeTextButton(const FText& Label, TFunction<void()> OnClicked, EFactoryButtonType Type = EFactoryButtonType::Normal, float FontSize = 20.0f, ETextJustify::Type Justification = ETextJustify::Left);

	/** Creates a rounded, semi transparent panel around the content */
	UBorder* MakePanel(UWidget* Content);

	/** Creates a solid color rectangle */
	UWidget* MakeColorBlock(const FLinearColor& Color, float Width = 0.0f, float Height = 0.0f);

	/** Creates vertical empty space */
	UWidget* MakeSpacer(float Height);

	/** Wraps the content in a size box with a fixed width and/or height (0 = unconstrained) */
	UWidget* MakeSized(UWidget* Content, float Width, float Height = 0.0f);

	/** Creates a settings row with a label and "<" value ">" buttons. OnStep receives -1 or +1 */
	UWidget* MakeCycleRow(const FText& Label, const FText& Value, TFunction<void(int32)> OnStep);

	/** Adds a child to a vertical box with the given padding */
	void AddRow(UVerticalBox* Box, UWidget* Child, const FMargin& RowPadding = FMargin(0.0f, 4.0f));

	/** Adds a child to a horizontal box */
	void AddColumn(UHorizontalBox* Box, UWidget* Child, bool bFill = false, const FMargin& ColumnPadding = FMargin(0.0f, 0.0f, 8.0f, 0.0f));

private:

	/** Root of the widget tree. Its content is replaced on every rebuild */
	UPROPERTY()
	TObjectPtr<UBorder> RootBorder;

	/** Button callbacks of the current layout */
	UPROPERTY()
	TArray<TObjectPtr<UFactoryMenuAction>> Actions;
};
