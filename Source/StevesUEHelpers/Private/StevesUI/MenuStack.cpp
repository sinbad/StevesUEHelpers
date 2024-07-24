#include "StevesUI/MenuStack.h"

#include "StevesUI.h"
#include "StevesGameSubsystem.h"
#include "StevesUEHelpers.h"
#include "StevesUI/MenuBase.h"
#include "Kismet/GameplayStatics.h"

void UMenuStack::NativeConstruct()
{
    Super::NativeConstruct();

    // We could technically do input change detection in our own input processor, but since this already does it nicely...
    auto GS = GetStevesGameSubsystem(GetWorld());
    if (GS)
    {
        GS->OnInputModeChanged.AddDynamic(this, &UMenuStack::InputModeChanged);
        LastInputMode = GS->GetLastInputModeUsed();
    }

    SavePreviousInputMousePauseState();

    ApplyInputModeChange(InputModeSettingOnOpen);
    ApplyMousePointerVisibility(MousePointerVisibilityOnOpen);
    ApplyGamePauseChange(GamePauseSettingOnOpen);
}

void UMenuStack::NativeDestruct()
{
    Super::NativeDestruct();

    auto GS = GetStevesGameSubsystem(GetWorld());
    if (GS)
        GS->OnInputModeChanged.RemoveDynamic(this, &UMenuStack::InputModeChanged);

}


void UMenuStack::SavePreviousInputMousePauseState()
{
    auto PC = GetOwningPlayer();    
    UGameViewportClient* GVC = GetWorld()->GetGameViewport();

    if (GVC->IgnoreInput())
    {
        PreviousInputMode = EInputModeChange::UIOnly;
    }
    else
    {
        
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 26) || ENGINE_MAJOR_VERSION >= 5			
		const auto CaptureMode = GVC->GetMouseCaptureMode();
#else
		const auto CaptureMode = GVC->CaptureMouseOnClick();
#endif

        // Game-only mode captures permanently, that seems to be the best way to detect
        if (CaptureMode == EMouseCaptureMode::CapturePermanently ||
            CaptureMode == EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown)
        {
            PreviousInputMode = EInputModeChange::GameOnly;
        }
        else
        {
            PreviousInputMode = EInputModeChange::GameAndUI;
        }
    }
    PreviousMouseVisibility = PC->bShowMouseCursor ? EMousePointerVisibilityChange::Visible : EMousePointerVisibilityChange::Hidden;
    PreviousPauseState = UGameplayStatics::IsGamePaused(GetWorld()) ? EGamePauseChange::Paused : EGamePauseChange::Unpaused;
    
    
}

void UMenuStack::ApplyInputModeChange(EInputModeChange Change) const
{
    auto PC = GetOwningPlayer();

    if (!PC) // possible during shutdown
        return;
    
    switch (Change)
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
    case EInputModeChange::Restore:
        ApplyInputModeChange(PreviousInputMode);
        break;
    }

    if (Change != EInputModeChange::DoNotChange && bFlushOnInputModeChange)
    {
        PC->FlushPressedKeys();
    }
}

void UMenuStack::ApplyMousePointerVisibility(EMousePointerVisibilityChange Change) const
{
    auto PC = GetOwningPlayer();    

    if (!PC) // possible during shutdown
        return;
    
    switch (Change)
    {
    case EMousePointerVisibilityChange::DoNotChange:
        break;
    case EMousePointerVisibilityChange::Visible:
        PC->bShowMouseCursor = true;
        break;
    case EMousePointerVisibilityChange::Hidden:
        PC->bShowMouseCursor = false;
        break;
    case EMousePointerVisibilityChange::Restore:
        ApplyMousePointerVisibility(PreviousMouseVisibility);
        break;
    }
}

void UMenuStack::ApplyGamePauseChange(EGamePauseChange Change) const
{
    switch (Change)
    {
    case EGamePauseChange::DoNotChange:
        break;
    case EGamePauseChange::Paused:
        UGameplayStatics::SetGamePaused(GetWorld(), true);
        break;
    case EGamePauseChange::Unpaused:
        UGameplayStatics::SetGamePaused(GetWorld(), false);
        break;
    case EGamePauseChange::Restore:
        ApplyGamePauseChange(PreviousPauseState);
        break;
    }    
    
}

bool UMenuStack::HandleKeyDownEvent(const FKeyEvent& InKeyEvent)
{
    Super::HandleKeyDownEvent(InKeyEvent);
    
    // Hardcoding the Back / Exit menu navigation inputs because input mappings can't be trusted in UMG
    // This is probably OK though, no-one redefines menu controls, right?
    const FKey Key = InKeyEvent.GetKey();
    if (BackKeys.Contains(Key))
    {
        // This is "Back"
        // Request close but allow veto
        if (Menus.Num() > 0)
        {
            auto Top = Menus.Last();
            Top->RequestClose(true);
        }
        return true;
    }
    else if (InstantCloseKeys.Contains(Key))
    {
        // Shortcut to exit all menus
        // Make sure we're open long enough
        FDateTime CurrTime = FDateTime::Now();
        FTimespan Diff = CurrTime - TimeFirstOpen;
        if (Diff.GetTicks() > (int64)(MinTimeOpen * ETimespan::TicksPerSecond))
        {
            CloseAll(true);
            return true;
        }
        
    }

    return false;
}


void UMenuStack::InputModeChanged(int PlayerIndex, EInputMode NewMode)
{
    if (Menus.Num())
    {
        Menus.Last()->InputModeChanged(LastInputMode, NewMode);
    }
    LastInputMode = NewMode;
}

UMenuBase* UMenuStack::PushMenuByClass(TSubclassOf<UMenuBase> MenuClass)
{
    const FName Name = MakeUniqueObjectName(this->GetOuter(), MenuClass);
    TSubclassOf<UUserWidget> BaseClass = MenuClass;
    const auto NewMenu = Cast<UMenuBase>(CreateWidgetInstance(*this, BaseClass, Name));
    PushMenuByObject(NewMenu);

    return NewMenu;
}

void UMenuStack::PushMenuByObject(UMenuBase* NewMenu)
{
    if (Menus.Num() > 0)
    {
        auto Top = Menus.Last();
        Top->SupercededInStack(NewMenu);
        // We keep this allocated, to restore later on back
    }
    Menus.Add(NewMenu);
    NewMenu->AddedToStack(this);

    if (Menus.Num() == 1)
        FirstMenuOpened();
}

void UMenuStack::PopMenu(bool bWasCancel)
{
    if (Menus.Num() > 0)
    {
        auto Top = Menus.Last();
        Top->RemovedFromStack(this);
        Menus.Pop();
        // No explicit destroy in UMG, let GC do it
        // we could try to re-use but probably not worth it vs complexity of reset for now

        if (Menus.Num() == 0)
        {
            LastMenuClosed(bWasCancel);
        }
        else
        {
            auto NewTop = Menus.Last();
            NewTop->RegainedFocusInStack();
        }
    }

}

void UMenuStack::PopMenuIfTop(UMenuBase* UiMenuBase, bool bWasCancel)
{
    if (Menus.Last() == UiMenuBase)
    {
        PopMenu(bWasCancel);
    }
    else
    {
        UE_LOG(LogStevesUI, Error, TEXT("Tried to pop menu %s but it wasn't the top level"), *UiMenuBase->GetName());
    }
}


void UMenuStack::FirstMenuOpened()
{
    // Don't use world time (even real time) since map can change while open
    TimeFirstOpen = FDateTime::Now();
}

void UMenuStack::RemoveFromParent()
{
    if (Menus.Num() > 0)
    {
        CloseAll(false);
        // LastMenuClosed will re-call this so don't duplicate
        return;
    }
    
    Super::RemoveFromParent();

}

UMenuBase* UMenuStack::GetTopMenu() const
{
    if (Menus.Num() > 0)
    {
        return Menus.Top();
    }
    return nullptr;
}

UMenuStack::UMenuStack()
{
    // Default to enabling automatic focus for menus (can still be overridden in serialized properties)
    bEnableAutomaticFocus = true;
}

void UMenuStack::LastMenuClosed(bool bWasCancel)
{
    RemoveFromParent(); // this will do MenuSystem interaction
    OnClosed.Broadcast(this, bWasCancel);

    ApplyInputModeChange(InputModeSettingOnClose);
    ApplyMousePointerVisibility(MousePointerVisibilityOnClose);
    ApplyGamePauseChange(GamePauseSettingOnClose);
    
}


void UMenuStack::CloseAll(bool bWasCancel)
{
    // We don't go through normal pop sequence, this is a shot circuit
    for (int i = Menus.Num() - 1; i >= 0; --i)
    {
        Menus[i]->RemovedFromStack(this);
    }
    Menus.Empty();
    LastMenuClosed(bWasCancel);
}

bool UMenuStack::IsRequestingFocus_Implementation() const
{
    // Delegate to top menu
    return Menus.Num() > 0 && Menus.Last()->IsRequestingFocus();
}

void UMenuStack::SetFocusProperly_Implementation()
{
    // Delegate to top menu
    if (Menus.Num() > 0)
        Menus.Last()->SetFocusProperly();
}
