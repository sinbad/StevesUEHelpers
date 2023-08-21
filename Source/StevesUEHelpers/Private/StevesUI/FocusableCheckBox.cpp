#include "StevesUI/FocusableCheckBox.h"
#include "StevesUI/SFocusableCheckBox.h"


TSharedRef<SWidget> UFocusableCheckBox::RebuildWidget()
{
    MyCheckbox = SNew(SFocusableCheckBox)
        .OnCheckStateChanged( BIND_UOBJECT_DELEGATE(FOnCheckStateChanged, SlateOnCheckStateChangedCallback) )
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
        .Style(&GetWidgetStyle())
        .ClickMethod(GetClickMethod())
        .TouchMethod(GetTouchMethod())
        .PressMethod(GetPressMethod())
#else
        .Style(&WidgetStyle)
        .ClickMethod(ClickMethod)
        .TouchMethod(TouchMethod)
        .PressMethod(PressMethod)
#endif
        .HAlign( HorizontalAlignment )
        .IsFocusable(GetIsFocusable())
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

    RefreshFocussedStyle();

    
    return MyCheckbox.ToSharedRef();
   
}

void UFocusableCheckBox::RefreshFocussedStyle_Implementation()
{
    // Copy Widget style but make normal same as hovered    
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
    FocussedStyle = GetWidgetStyle();
#else
    FocussedStyle = WidgetStyle;
#endif
    FocussedStyle.UncheckedImage = FocussedStyle.UncheckedHoveredImage;
    FocussedStyle.CheckedImage = FocussedStyle.CheckedHoveredImage;
    FocussedStyle.UndeterminedImage = FocussedStyle.UndeterminedHoveredImage;
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
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 0
        MyCheckbox->SetStyle(&GetWidgetStyle());
#else
        MyCheckbox->SetStyle(&WidgetStyle);
#endif
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
