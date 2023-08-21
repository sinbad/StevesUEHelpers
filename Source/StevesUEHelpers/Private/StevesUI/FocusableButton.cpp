#include "StevesUI/FocusableButton.h"
#include "StevesUI/SFocusableButton.h"
#include "Components/ButtonSlot.h"

UFocusableButton::UFocusableButton(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UFocusableButton::RebuildWidget()
{
    MyButton = SNew(SFocusableButton)
            // This part is standard button behaviour
            .OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleClicked))
            .OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandlePressed))
            .OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleReleased))
            .OnHovered_UObject( this, &ThisClass::SlateHandleHovered )
            .OnUnhovered_UObject( this, &ThisClass::SlateHandleUnhovered )
            .ButtonStyle(&GetStyle())
            .ClickMethod(GetClickMethod())
            .TouchMethod(GetTouchMethod())
            .PressMethod(GetPressMethod())
            .IsFocusable(GetIsFocusable())
            // Our new events
            .OnFocusReceived(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleFocusReceived))
            .OnFocusLost(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleFocusLost))
    ;

    if ( GetChildrenCount() > 0 )
    {
        Cast<UButtonSlot>(GetContentSlot())->BuildSlot(MyButton.ToSharedRef());
    }

    RefreshFocussedStyle();

    return MyButton.ToSharedRef();    
}

void UFocusableButton::RefreshFocussedStyle_Implementation()
{
    // Copy Widget style but make normal same as hovered    
    FocussedStyle = GetStyle();
    FocussedStyle.Normal = FocussedStyle.Hovered;
}

void UFocusableButton::SlateHandleFocusReceived()
{
    ApplyFocusStyle();
    OnFocusReceived.Broadcast();
}

void UFocusableButton::SlateHandleFocusLost()
{
    UndoFocusStyle();
    OnFocusLost.Broadcast();
}


void UFocusableButton::ApplyFocusStyle()
{
    if (MyButton.IsValid() && bUseHoverStyleWhenFocussed)
    {
        MyButton->SetButtonStyle(&FocussedStyle);
    }
}

void UFocusableButton::UndoFocusStyle()
{
    if (MyButton.IsValid())
    {
        MyButton->SetButtonStyle(&GetStyle());
    }
}

void UFocusableButton::SlateHandleHovered()
{
    if (bTakeFocusOnHover)
    {
        SetFocus();
    }
    OnHovered.Broadcast();
}

void UFocusableButton::SlateHandleUnhovered()
{
    OnUnhovered.Broadcast();
}
