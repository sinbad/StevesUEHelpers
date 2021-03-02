#include "StevesUI/FocusableCheckBox.h"
#include "StevesUI/SFocusableCheckBox.h"


TSharedRef<SWidget> UFocusableCheckBox::RebuildWidget()
{
    MyCheckbox = SNew(SFocusableCheckBox)
        .OnCheckStateChanged( BIND_UOBJECT_DELEGATE(FOnCheckStateChanged, SlateOnCheckStateChangedCallback) )
        .Style(&WidgetStyle)
        .HAlign( HorizontalAlignment )
        .ClickMethod(ClickMethod)
        .TouchMethod(TouchMethod)
        .PressMethod(PressMethod)
        .IsFocusable(IsFocusable)
        // Our new events
        .OnHovered(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleHovered))
        .OnUnhovered(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleUnhovered))
        .OnFocusReceived(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleFocusReceived))
        .OnFocusLost(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleFocusLost))
    ;

    if ( GetChildrenCount() > 0 )
    {
        MyCheckbox->SetContent(GetContentSlot()->Content ? GetContentSlot()->Content->TakeWidget() : SNullWidget::NullWidget);
    }

    // Copy Widget style but make normal same as hovered    
    FocussedStyle = WidgetStyle;
    FocussedStyle.UncheckedImage = FocussedStyle.UncheckedHoveredImage;
    FocussedStyle.CheckedImage = FocussedStyle.CheckedHoveredImage;
    FocussedStyle.UndeterminedImage = FocussedStyle.UndeterminedHoveredImage;
    
    return MyCheckbox.ToSharedRef();
   
}

void UFocusableCheckBox::SlateHandleFocusReceived()
{
    ApplyFocusStyle();
    OnFocusReceived.Broadcast();
}

void UFocusableCheckBox::SlateHandleFocusLost()
{
    UndoFocusStyle();
    OnFocusLost.Broadcast();
}


void UFocusableCheckBox::ApplyFocusStyle()
{
    if (MyCheckbox.IsValid() && bUseHoverStyleWhenFocussed)
    {
        MyCheckbox->SetStyle(&FocussedStyle);
    }
}

void UFocusableCheckBox::UndoFocusStyle()
{
    if (MyCheckbox.IsValid())
    {
        MyCheckbox->SetStyle(&WidgetStyle);
    }
}

void UFocusableCheckBox::SlateHandleHovered()
{
    if (bTakeFocusOnHover)
    {
        SetFocus();
    }
    OnHovered.Broadcast();
}

void UFocusableCheckBox::SlateHandleUnhovered()
{
    OnUnhovered.Broadcast();
}
