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
    Open(true);
    
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
    if (ParentStack.IsValid() && bEmbedInParentContainer)
        EmbedInParent();
    else
        AddToViewport();
    SetVisibility(ESlateVisibility::Visible);

    auto PC = GetOwningPlayer();    
    switch (InputModeSetting)
    {
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
