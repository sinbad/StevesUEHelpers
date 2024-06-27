#include "StevesUI/FocusSystem.h"
#include "StevesUI/FocusableUserWidget.h"

DEFINE_LOG_CATEGORY(LogFocusSystem)

TWeakObjectPtr<UFocusableUserWidget> FFocusSystem::GetHighestFocusPriority()
{
    int Highest = -999;
    TWeakObjectPtr<UFocusableUserWidget> Ret;
    
    for (auto && S : ActiveAutoFocusWidgets)
    {
        if (S.IsValid() && S->IsRequestingFocus() && S->GetAutomaticFocusPriority() > Highest)
        {
            Highest = S->GetAutomaticFocusPriority();
            Ret = S;
        }
    }
        
    return Ret;
}

void FFocusSystem::FocusableWidgetConstructed(UFocusableUserWidget* Widget)
{
    UE_LOG(LogFocusSystem, Display, TEXT("FocusableUserWidget %s opened"), *Widget->GetName());
    // check to make sure we never dupe, shouldn't normally be a problem
    // but let's just be safe, there will never be that many
    bool bPresent = false;    
    for (auto && S : ActiveAutoFocusWidgets)
    {
        if (S.Get() == Widget)
        {
            bPresent = true;
            break;
        }
    }
    if (!bPresent)
        ActiveAutoFocusWidgets.Add(Widget);

    if (Widget->IsRequestingFocus())
    {
        auto Highest = GetHighestFocusPriority();
        if (!Highest.IsValid() || Highest->GetAutomaticFocusPriority() <= Widget->GetAutomaticFocusPriority())
        {
            // give new stack the focus if it's equal or higher priority than anything else
            UE_LOG(LogFocusSystem, Display, TEXT("Giving focus to %s"), *Widget->GetName());
            Widget->TakeFocusIfDesired();
        }        
    }
}

void FFocusSystem::FocusableWidgetDestructed(UFocusableUserWidget* Widget)
{
    UE_LOG(LogFocusSystem, Display, TEXT("FocusableUserWidget %s closed"), *Widget->GetName());
    
    for (int i = 0; i < ActiveAutoFocusWidgets.Num(); ++i)
    {
        if (ActiveAutoFocusWidgets[i].Get() == Widget)
        {
            ActiveAutoFocusWidgets.RemoveAt(i);
            break;
        }
    }

    // if the menu closing had focus, give it to the highest remaining stack
    if (Widget->HasFocusedDescendants())
    {
        auto Highest = GetHighestFocusPriority();
        // Make sure player controller is valid too, this could be on shutdown
        if (Highest.IsValid() && Highest->GetOwningPlayer())
        {
            UE_LOG(LogFocusSystem, Display, TEXT("Giving focus to %s"), *Highest->GetName());
            Highest->TakeFocusIfDesired();
        }       
    }
}
