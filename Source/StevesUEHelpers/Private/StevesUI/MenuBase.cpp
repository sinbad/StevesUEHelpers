#include "StevesUI/MenuBase.h"

#include "StevesUI.h"
#include "StevesGameSubsystem.h"
#include "StevesUEHelpers.h"
#include "StevesUI/MenuStack.h"
#include "Components/ContentWidget.h"
#include "Kismet/GameplayStatics.h"

void UMenuBase::Close(bool bWasCancel)
{
    // Deliberately raise before parent so stack is always last in event sequence
    OnClosed.Broadcast(this, bWasCancel);
    if (ParentStack.IsValid())
    {
        ParentStack->PopMenuIfTop(this, bWasCancel);
    } else
    {
        // standalone mode
        RemoveFromParent();
        PreviousFocusWidget.Reset();
    }
}

void UMenuBase::AddedToStack(UMenuStack* Parent)
{
    ParentStack = MakeWeakObjectPtr(Parent);

    Open(false);
    OnAddedToStack(Parent);
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

bool UMenuBase::IsTopOfStack() const
{
    if (ParentStack.IsValid())
    {
        return ParentStack->GetTopMenu() == this;
    }

    return true;
}

void UMenuBase::RemovedFromStack(UMenuStack* Parent)
{
    // This works whether embedded or not
    RemoveFromParent();
    PreviousFocusWidget.Reset();
    OnRemovedFromStack(Parent);
}

void UMenuBase::SupercededInStack(UMenuBase* ByMenu)
{
    SavePreviousFocus();
    
    if (bEmbedInParentContainer)
    {
        if (bHideWhenSuperceded)
            RemoveFromParent();
    }
    else
    {
        if (bHideWhenSuperceded)
            SetVisibility(ESlateVisibility::Collapsed);
    }
    OnSupercededInStack(ByMenu);
}

void UMenuBase::RegainedFocusInStack()
{
    Open(true);
    OnRegainedFocusInStack();
}

void UMenuBase::EmbedInParent()
{
    if (ParentStack.IsValid() &&
        ParentStack->MenuContainer != nullptr)
    {
        ParentStack->MenuContainer->SetContent(this);
    }
    else
        UE_LOG(LogStevesUI, Error, TEXT("Cannot embed %s in parent, missing container"), *this->GetName())
    
}

void UMenuBase::Open(bool bIsRegain)
{
    if (ParentStack.IsValid() && bEmbedInParentContainer && bHideWhenSuperceded)
        EmbedInParent();
    else
        AddToViewport();
    SetVisibility(ESlateVisibility::Visible);

    auto PC = GetOwningPlayer();    
    switch (InputModeSetting)
    {
    default:
    case EInputModeChange::DoNotChange:
        break;
    case EInputModeChange::UIOnly:
        PC->SetInputMode(FInputModeUIOnly());
        break;
    case EInputModeChange::GameAndUI:
        PC->SetInputMode(FInputModeGameAndUI());
        break;
    case EInputModeChange::GameOnly:
        PC->SetInputMode(FInputModeGameOnly());
        break;
    }

    switch (MousePointerVisibility)
    {
    default:
    case EMousePointerVisibilityChange::DoNotChange:
        break;
    case EMousePointerVisibilityChange::Visible:
        PC->bShowMouseCursor = true;
        break;
    case EMousePointerVisibilityChange::Hidden:
        PC->bShowMouseCursor = false;
        break;
    }

    switch (GamePauseSetting)
    {
    default:
    case EGamePauseChange::DoNotChange:
        break;
    case EGamePauseChange::Paused:
        UGameplayStatics::SetGamePaused(GetWorld(), true);
        break;
    case EGamePauseChange::Unpaused:
        UGameplayStatics::SetGamePaused(GetWorld(), false);
        break;
    }    

    TakeFocusIfDesired();
    
}

bool UMenuBase::RequestClose(bool bWasCancel)
{
    if (ValidateClose(bWasCancel))
    {
        Close(bWasCancel);
        return true;
    }
    return false;
}

bool UMenuBase::ValidateClose_Implementation(bool bWasCancel)
{
    // Default always pass
    return true;
}

void UMenuBase::OnSupercededInStack_Implementation(UMenuBase* ByMenu)
{
}

void UMenuBase::OnRegainedFocusInStack_Implementation()
{
}

void UMenuBase::OnAddedToStack_Implementation(UMenuStack* Parent)
{
}

void UMenuBase::OnRemovedFromStack_Implementation(UMenuStack* Parent)
{
}
