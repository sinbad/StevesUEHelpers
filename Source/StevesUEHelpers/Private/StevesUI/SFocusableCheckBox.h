#pragma once

#include "CoreMinimal.h"
#include "Widgets/Input/SCheckBox.h"


class SFocusableCheckBox : public SCheckBox
{
	public:

	// Seems in slate we have to duplicate all the args from superclass
	SLATE_BEGIN_ARGS( SFocusableCheckBox )
        : _Content()
        , _Style( &FCoreStyle::Get().GetWidgetStyle< FCheckBoxStyle >("Checkbox") )
        , _Type()
        , _OnCheckStateChanged()
        , _IsChecked( ECheckBoxState::Unchecked )
        , _HAlign( HAlign_Fill )
        , _CheckBoxContentUsesAutoWidth(true)
        , _Padding()
        , _ClickMethod( EButtonClickMethod::DownAndUp )
        , _TouchMethod(EButtonTouchMethod::DownAndUp)
        , _PressMethod(EButtonPressMethod::DownAndUp)
        , _ForegroundColor(FSlateColor(FColor::White))
        , _BorderBackgroundColor (FSlateColor(FColor::White))
        , _IsFocusable( true )
        , _UncheckedImage( nullptr )
        , _UncheckedHoveredImage( nullptr )
        , _UncheckedPressedImage( nullptr )
        , _CheckedImage( nullptr )
        , _CheckedHoveredImage( nullptr )
        , _CheckedPressedImage( nullptr )
        , _UndeterminedImage( nullptr )
        , _UndeterminedHoveredImage( nullptr )
        , _UndeterminedPressedImage( nullptr )
	{
	}
		/** Content to be placed next to the check box, or for a toggle button, the content to be placed inside the button */
		SLATE_DEFAULT_SLOT( FArguments, Content )

		/** The style structure for this checkbox' visual style */
		SLATE_STYLE_ARGUMENT( FCheckBoxStyle, Style )

		/** Type of check box (set by the Style arg but the Style can be overridden with this) */
		SLATE_ARGUMENT( TOptional<ESlateCheckBoxType::Type>, Type )

		/** Called when the checked state has changed */
		SLATE_EVENT( FOnCheckStateChanged, OnCheckStateChanged )

		/** Whether the check box is currently in a checked state */
		SLATE_ATTRIBUTE( ECheckBoxState, IsChecked )

		/** How the content of the toggle button should align within the given space*/
		SLATE_ARGUMENT( EHorizontalAlignment, HAlign )

		/** Whether or not the content portion of the checkbox should layout using auto-width. When true the content will always be arranged at its desired size as opposed to resizing to the available space. */
		SLATE_ARGUMENT(bool, CheckBoxContentUsesAutoWidth)

		/** Spacing between the check box image and its content (set by the Style arg but the Style can be overridden with this) */
		SLATE_ATTRIBUTE( FMargin, Padding )

		/** Sets the rules to use for determining whether the button was clicked.  This is an advanced setting and generally should be left as the default. */
		SLATE_ARGUMENT( EButtonClickMethod::Type, ClickMethod )

		/** How should the button be clicked with touch events? */
		SLATE_ARGUMENT(EButtonTouchMethod::Type, TouchMethod)

		/** How should the button be clicked with keyboard/controller button events? */
		SLATE_ARGUMENT(EButtonPressMethod::Type, PressMethod)

		/** Foreground color for the checkbox's content and parts (set by the Style arg but the Style can be overridden with this) */
		SLATE_ATTRIBUTE( FSlateColor, ForegroundColor )

		/** The color of the background border (set by the Style arg but the Style can be overridden with this) */
		SLATE_ATTRIBUTE( FSlateColor, BorderBackgroundColor )

		SLATE_ARGUMENT( bool, IsFocusable )
		
		SLATE_EVENT( FOnGetContent, OnGetMenuContent )

		/** The sound to play when the check box is checked */
		SLATE_ARGUMENT( TOptional<FSlateSound>, CheckedSoundOverride )

		/** The sound to play when the check box is unchecked */
		SLATE_ARGUMENT( TOptional<FSlateSound>, UncheckedSoundOverride )

		/** The sound to play when the check box is hovered */
		SLATE_ARGUMENT( TOptional<FSlateSound>, HoveredSoundOverride )

		/** The unchecked image for the checkbox - overrides the style's */
		SLATE_ARGUMENT(const FSlateBrush*, UncheckedImage)

		/** The unchecked hovered image for the checkbox - overrides the style's */
		SLATE_ARGUMENT(const FSlateBrush*, UncheckedHoveredImage)

		/** The unchecked pressed image for the checkbox - overrides the style's */
		SLATE_ARGUMENT(const FSlateBrush*, UncheckedPressedImage)

		/** The checked image for the checkbox - overrides the style's */
		SLATE_ARGUMENT(const FSlateBrush*, CheckedImage)

		/** The checked hovered image for the checkbox - overrides the style's */
		SLATE_ARGUMENT(const FSlateBrush*, CheckedHoveredImage)

		/** The checked pressed image for the checkbox - overrides the style's */
		SLATE_ARGUMENT(const FSlateBrush*, CheckedPressedImage)

		/** The undetermined image for the checkbox - overrides the style's */
		SLATE_ARGUMENT(const FSlateBrush*, UndeterminedImage)

		/** The undetermined hovered image for the checkbox - overrides the style's */
		SLATE_ARGUMENT(const FSlateBrush*, UndeterminedHoveredImage)

		/** The undetermined pressed image for the checkbox - overrides the style's */
		SLATE_ARGUMENT(const FSlateBrush*, UndeterminedPressedImage)

	    // This is the bit we're adding
		// SCheckBox is missing hovered/unhovered too
		SLATE_EVENT( FSimpleDelegate, OnHovered )
	    SLATE_EVENT( FSimpleDelegate, OnUnhovered )
		SLATE_EVENT( FSimpleDelegate, OnFocusReceived )
	    SLATE_EVENT( FSimpleDelegate, OnFocusLost )
	
	SLATE_END_ARGS()

public:

	void Construct( const FArguments& InArgs );
    
	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;
	virtual void OnFocusLost(const FFocusEvent& InFocusEvent) override;

	void SetOnFocusReceived(FSimpleDelegate InOnFocusReceived);
	void SetOnFocusLost(FSimpleDelegate InOnFocusLost);

protected:
	FSimpleDelegate OnHoveredDelegate;
	FSimpleDelegate OnUnhoveredDelegate;
	FSimpleDelegate OnFocusReceivedDelegate;
	FSimpleDelegate OnFocusLostDelegate;


public:
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
};