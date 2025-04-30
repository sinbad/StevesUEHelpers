#include "SFocusableSlider.h"

void SFocusableSlider::Construct(const FArguments& InArgs)
{
	// Call superclass, have to re-construct the args because they're not compatible
	SSlider::Construct(SSlider::FArguments()
	                   .Locked(InArgs._Locked)
	                   .Orientation(InArgs._Orientation)
	                   .Style(InArgs._Style)
	                   .Value(InArgs._Value)
	                   .MaxValue(InArgs._MaxValue)
	                   .MinValue(InArgs._MinValue)
	                   .IsFocusable(InArgs._IsFocusable)
	                   .IsEnabled(InArgs._IsEnabled)
	                   .IndentHandle(InArgs._IndentHandle)
	                   .MouseUsesStep(InArgs._MouseUsesStep)
	                   .RequiresControllerLock(InArgs._RequiresControllerLock)
	                   .Clipping(InArgs._Clipping)
	                   .Cursor(InArgs._Cursor)
	                   .Tag(InArgs._Tag)
	                   .Visibility(InArgs._Visibility)
	                   .AccessibleParams(InArgs._AccessibleParams)
	                   .AccessibleText(InArgs._AccessibleText)
	                   .ForceVolatile(InArgs._ForceVolatile)
	                   .RenderOpacity(InArgs._RenderOpacity)
	                   .RenderTransform(InArgs._RenderTransform)
	                   .RenderTransformPivot(InArgs._RenderTransformPivot)
	                   .ToolTip(InArgs._ToolTip)
	                   .ToolTipText(InArgs._ToolTipText)
	                   .OnValueChanged(InArgs._OnValueChanged)
		);

	OnHoveredDelegate = InArgs._OnHovered;
	OnUnhoveredDelegate = InArgs._OnUnhovered;
	OnFocusReceivedDelegate = InArgs._OnFocusReceived;
	OnFocusLostDelegate = InArgs._OnFocusLost;
}


void SFocusableSlider::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SSlider::OnMouseEnter(MyGeometry, MouseEvent);

	// SSlider doesn't have hovered / unhovered events so we need to add them
	OnHoveredDelegate.ExecuteIfBound();
}

void SFocusableSlider::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SSlider::OnMouseLeave(MouseEvent);

	OnUnhoveredDelegate.ExecuteIfBound();
}

FReply SFocusableSlider::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	auto Ret = SSlider::OnFocusReceived(MyGeometry, InFocusEvent);

	OnFocusReceivedDelegate.ExecuteIfBound();

	return Ret;
}

void SFocusableSlider::OnFocusLost(const FFocusEvent& InFocusEvent)
{
	SSlider::OnFocusLost(InFocusEvent);

	OnFocusLostDelegate.ExecuteIfBound();
}

void SFocusableSlider::SetOnFocusReceived(FSimpleDelegate InOnFocusReceived)
{
	OnFocusReceivedDelegate = InOnFocusReceived;
}

void SFocusableSlider::SetOnFocusLost(FSimpleDelegate InOnFocusLost)
{
	OnFocusLostDelegate = InOnFocusLost;
}
