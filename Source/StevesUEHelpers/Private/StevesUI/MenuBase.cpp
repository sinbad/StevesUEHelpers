#include "StevesUI/MenuBase.h"

#include "StevesUI.h"
#include "StevesGameSubsystem.h"
#include "StevesUEHelpers.h"
#include "StevesUI/MenuStack.h"
#include "Components/ContentWidget.h"

void UMenuBase::Close(bool bWasCancel)
{
    // Deliberately raise before parent so stack is always last in event sequence
    OnClosed.Broadcast(this, bWasCancel);
    if (ParentStack.IsValid())
    {
        ParentStack->PopMenuIfTop(this, bWasCancel);
    }
}

void UMenuBase::AddedToStack(UMenuStack* Parent)
{
    ParentStack = MakeWeakObjectPtr(Parent);
    if (bEmbedInParentContainer)
        EmbedInParent();
    else
        AddToViewport();
    SetVisibility(ESlateVisibility::Visible);

    auto GS = GetStevesGameSubsystem(GetWorld());
    if (bRequestFocus &&
        GS && (GS->GetLastInputModeUsed() != EInputMode::Gamepad || GS->GetLastInputModeUsed() != EInputMode::Keyboard))
    {
        SetFocusProperly();
    }
        
    
}


void UMenuBase::InputModeChanged(EInputMode OldMode, EInputMode NewMode)
{
    if (OldMode == EInputMode::Mouse &&
        (NewMode == EInputMode::Gamepad || NewMode == EInputMode::Keyboard))
    {
        if (bRequestFocus)
            SetFocusProperly();
    }
    else if ((OldMode == EInputMode::Gamepad || OldMode == EInputMode::Keyboard) &&
        NewMode == EInputMode::Mouse)
    {
        SavePreviousFocus();
    }
}

void UMenuBase::RemovedFromStack(UMenuStack* Parent)
{
    // This works whether embedded or not
    RemoveFromParent();
    PreviousFocusWidget.Reset();
}

void UMenuBase::SupercededInStack()
{
    SavePreviousFocus();
    
    if (bEmbedInParentContainer)
        RemoveFromParent();
    else
    {
        if (bHideWhenSuperceded)
            SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UMenuBase::RegainedFocusInStack()
{
    if (bEmbedInParentContainer)
        EmbedInParent();
    else
        AddToViewport();
    SetVisibility(ESlateVisibility::Visible);

    if (bRequestFocus)
    {
        SetFocusProperly();
    }
    
}

void UMenuBase::EmbedInParent()
{
    if (ParentStack.IsValid() &&
        ParentStack->MenuContainer != nullptr)
    {
        ParentStack->MenuContainer->SetContent(this);
    }
    else
        UE_LOG(LogCustomUI, Error, TEXT("Cannot embed %s in parent, missing container"), *this->GetName())
    
}
