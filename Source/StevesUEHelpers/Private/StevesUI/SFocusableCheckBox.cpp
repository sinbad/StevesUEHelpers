#include "SFocusableCheckBox.h"

void SFocusableCheckBox::Construct(const FArguments& InArgs)
{
	// Call superclass, have to re-construct the args because they're not compatible
	SCheckBox::Construct(SCheckBox::FArguments()
		.Padding(InArgs._Padding)
		.Style(InArgs._Style)
		.Type(InArgs._Type)
		.CheckedImage(InArgs._CheckedImage)
		.ClickMethod(InArgs._ClickMethod)
		.ForegroundColor(InArgs._ForegroundColor)
		.HAlign(InArgs._HAlign)
		.IsChecked(InArgs._IsChecked)
		.IsFocusable(InArgs._IsFocusable)
		.PressMethod(InArgs._PressMethod)
		.TouchMethod(InArgs._TouchMethod)
		.UncheckedImage(InArgs._UncheckedImage)
		.UndeterminedImage(InArgs._UndeterminedImage)
		.CheckedImage(InArgs._CheckedImage)
		.UncheckedHoveredImage(InArgs._UncheckedHoveredImage)
        .UndeterminedHoveredImage(InArgs._UndeterminedHoveredImage)
        .CheckedHoveredImage(InArgs._CheckedHoveredImage)
        .UncheckedPressedImage(InArgs._UncheckedPressedImage)
        .UndeterminedPressedImage(InArgs._UndeterminedPressedImage)
        .CheckedPressedImage(InArgs._CheckedPressedImage)
        .CheckedSoundOverride(InArgs._CheckedSoundOverride)
        .UncheckedSoundOverride(InArgs._UncheckedSoundOverride)
        .HoveredSoundOverride(InArgs._HoveredSoundOverride)
        .BorderBackgroundColor(InArgs._BorderBackgroundColor)
        .OnCheckStateChanged(InArgs._OnCheckStateChanged)
        .IsEnabled(InArgs._IsEnabled)
        .Clipping(InArgs._Clipping)
        .Cursor(InArgs._Cursor)
        .Tag(InArgs._Tag)
        .Visibility(InArgs._Visibility)
        .AccessibleParams(InArgs._AccessibleParams)
        .AccessibleText(InArgs._AccessibleText)
        .ForceVolatile(InArgs._ForceVolatile)
        .IsEnabled(InArgs._IsEnabled)
        .RenderOpacity(InArgs._RenderOpacity)
        .RenderTransform(InArgs._RenderTransform)
        .RenderTransformPivot(InArgs._RenderTransformPivot)
        .ToolTip(InArgs._ToolTip)
        .ToolTipText(InArgs._ToolTipText)		
	);

	OnHoveredDelegate = InArgs._OnHovered;
	OnUnhoveredDelegate = InArgs._OnUnhovered;
	OnFocusReceivedDelegate = InArgs._OnFocusReceived;
	OnFocusLostDelegate = InArgs._OnFocusLost;
	
	
}


FReply SFocusableCheckBox::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// Default 4.26.1 behaviour is to return Unhandled for ANY keys apart from Accept, which breaks keyboard nav
	// This is a bug and it's been fixed in the source version but not released yet, so work around it
	if (FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Accept)
	{
		// for accept, let the immediate superclass do it
		return SCheckBox::OnKeyDown(MyGeometry, InKeyEvent);
	}
	else
	{
		// for any other key, the super-super needs to handle it to support navigation
		return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
	}
}

void SFocusableCheckBox::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SCheckBox::OnMouseEnter(MyGeometry, MouseEvent);

	// SCheckbox doesn't have hovered / unhovered events so we need to add them
	OnHoveredDelegate.ExecuteIfBound();
}

void SFocusableCheckBox::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SCheckBox::OnMouseLeave(MouseEvent);

	OnUnhoveredDelegate.ExecuteIfBound();
}

FReply SFocusableCheckBox::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	auto Ret = SCheckBox::OnFocusReceived(MyGeometry, InFocusEvent);

	OnFocusReceivedDelegate.ExecuteIfBound();

	return Ret;
}

void SFocusableCheckBox::OnFocusLost(const FFocusEvent& InFocusEvent)
{
	SCheckBox::OnFocusLost(InFocusEvent);

	OnFocusLostDelegate.ExecuteIfBound();
}

void SFocusableCheckBox::SetOnFocusReceived(FSimpleDelegate InOnFocusReceived)
{
	OnFocusReceivedDelegate = InOnFocusReceived;
}

void SFocusableCheckBox::SetOnFocusLost(FSimpleDelegate InOnFocusLost)
{
	OnFocusLostDelegate = InOnFocusLost;
}
