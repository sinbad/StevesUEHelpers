#include "StevesUI/SFocusableButton.h"

void SFocusableButton::Construct(const FArguments& InArgs)
{
    // Call SButton construct and pass through
    SButton::Construct(SButton::FArguments()
        .ButtonStyle(InArgs._ButtonStyle)
        .ClickMethod(InArgs._ClickMethod)
        .ContentPadding(InArgs._ContentPadding)
        .ContentScale(InArgs._ContentScale)
        .FlowDirectionPreference(InArgs._FlowDirectionPreference)
        .ForegroundColor(InArgs._ForegroundColor)
        .HAlign(InArgs._HAlign)
        .VAlign(InArgs._VAlign)
        .IsFocusable(InArgs._IsFocusable)
        .OnClicked(InArgs._OnClicked)
        .OnHovered(InArgs._OnHovered)
        .OnPressed(InArgs._OnPressed)
        .OnReleased(InArgs._OnReleased)
        .OnUnhovered(InArgs._OnUnhovered)
        .PressMethod(InArgs._PressMethod)
        .TouchMethod(InArgs._TouchMethod)
        .DesiredSizeScale(InArgs._DesiredSizeScale)
        .HoveredSoundOverride(InArgs._HoveredSoundOverride)
        .PressedSoundOverride(InArgs._PressedSoundOverride)
        .ButtonColorAndOpacity(InArgs._ButtonColorAndOpacity)
        .TextFlowDirection(InArgs._TextFlowDirection)
        .TextShapingMethod(InArgs._TextShapingMethod)
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
        .Text(InArgs._Text)
        .TextStyle(InArgs._TextStyle)
        .ToolTip(InArgs._ToolTip)
        .ToolTipText(InArgs._ToolTipText)        
        );

    OnFocusReceivedDelegate = InArgs._OnFocusReceived;
    OnFocusLostDelegate = InArgs._OnFocusLost;
}

FReply SFocusableButton::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
    auto Ret = SButton::OnFocusReceived(MyGeometry, InFocusEvent);

    OnFocusReceivedDelegate.ExecuteIfBound();

    return Ret;
}

void SFocusableButton::OnFocusLost(const FFocusEvent& InFocusEvent)
{
    SButton::OnFocusLost(InFocusEvent);

    OnFocusLostDelegate.ExecuteIfBound();
}

void SFocusableButton::SetOnFocusReceived(FSimpleDelegate InOnFocusReceived)
{
    OnFocusReceivedDelegate = InOnFocusReceived;
}

void SFocusableButton::SetOnFocusLost(FSimpleDelegate InOnFocusLost)
{
    OnFocusLostDelegate = InOnFocusLost;
}

