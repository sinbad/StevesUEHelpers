#include "StevesUI/MenuStack.h"

#include "StevesUI.h"
#include "StevesGameSubsystem.h"
#include "StevesUEHelpers.h"
#include "StevesUI/MenuBase.h"
#include "Framework/Application/SlateApplication.h"


void UMenuStack::NativeConstruct()
{
    Super::NativeConstruct();

    if (!InputPreprocessor.IsValid())
    {
        InputPreprocessor = MakeShareable(new FUiInputPreprocessor());
        InputPreprocessor->OnUiKeyDown.BindUObject(this, &UMenuStack::HandleKeyDownEvent);
    }
    FSlateApplication::Get().RegisterInputPreProcessor(InputPreprocessor);

    // We could technically do input change detection in our own input processor, but since this already does it nicely...
    auto GS = GetStevesGameSubsystem(GetWorld());
    if (GS)
    {
        GS->OnInputModeChanged.AddDynamic(this, &UMenuStack::InputModeChanged);
        LastInputMode = GS->GetLastInputModeUsed();
    }
}

void UMenuStack::NativeDestruct()
{
    Super::NativeDestruct();
    FSlateApplication::Get().UnregisterInputPreProcessor(InputPreprocessor);

    auto GS = GetStevesGameSubsystem(GetWorld());
    if (GS)
        GS->OnInputModeChanged.RemoveDynamic(this, &UMenuStack::InputModeChanged);

    
}

bool UMenuStack::HandleKeyDownEvent(const FKeyEvent& InKeyEvent)
{
    // Hardcoding the Back / Exit menu navigation inputs because input mappings can't be trusted in UMG
    // This is probably OK though, no-one redefines menu controls, right?
    FKey Key = InKeyEvent.GetKey();
    if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right)
    {
        // This is "Back"
        PopMenu(true);
        return true;
    }
    else if (Key == EKeys::Gamepad_Special_Right)
    {
        // Shortcut to exit all menus
        CloseAll(true);
        return true;
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
    const FName Name = MakeUniqueObjectName(this->GetOuter(), MenuClass, FName("Menu"));
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
        Top->SupercededInStack();
        // We keep this allocated, to restore later on back
    }
    Menus.Add(NewMenu);
    NewMenu->AddedToStack(this);
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

void UMenuStack::LastMenuClosed(bool bWasCancel)
{
    RemoveFromParent();
    OnClosed.Broadcast(this, bWasCancel);
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

void UMenuStack::SetFocusProperly_Implementation()
{
    // Delegate to top menu
    if (Menus.Num() > 0)
        Menus.Last()->SetFocusProperly();
}
