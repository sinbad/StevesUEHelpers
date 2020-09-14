#pragma once

#include "CoreMinimal.h"

#include "FocusableUserWidget.h"
#include "Framework/Application/IInputProcessor.h"
#include "StevesHelperCommon.h"
#include "UObject/ObjectMacros.h"


#include "MenuStack.generated.h"

class UContentWidget;
class UMenuBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMenuStackClosed, class UMenuStack*, Stack, bool, bWasCancel);

/// Represents a modal stack of menus which take focus and have a concept of "Back"
/// Each level is a MenuBase
/// You can style this widget to be the general surrounds in which all MenuBase levels live inside
/// Create a Blueprint subclass of this and make sure you include a UContentWidget with the name
/// "MenuContainer" somewhere in the tree, which is where the menu contents will be placed. 
UCLASS(Abstract, BlueprintType)
class STEVESUEHELPERS_API UMenuStack : public UFocusableUserWidget
{
    GENERATED_BODY()

    // Nested class which we'll use to poke into input events before anything else eats them
    // Without this it seems impossible to pick up e.g. Gamepad "B" button with UMG up
    // It means we hardcode the controls for menus but that's OK in practice
    class FUiInputPreprocessor : public IInputProcessor, public TSharedFromThis<FUiInputPreprocessor>
    {
    public:
        DECLARE_DELEGATE_RetVal_OneParam(bool, FOnUiKeyDown, const FKeyEvent&);
        FOnUiKeyDown OnUiKeyDown;
        
        virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
        {
            return OnUiKeyDown.Execute(InKeyEvent);
        }
        // Required by IInputProcessor but we don't need
        virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Crs) override{}
    };

protected:
    EInputMode LastInputMode;
    
    TArray<UMenuBase*> Menus;

    TSharedPtr<FUiInputPreprocessor> InputPreprocessor;

    void LastMenuClosed(bool bWasCancel);

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    UFUNCTION()
    bool HandleKeyDownEvent(const FKeyEvent& InKeyEvent);
    UFUNCTION()
    void InputModeChanged(int PlayerIndex, EInputMode NewMode);

public:
    /// This property will bind to a blueprint variable of the same name to contain the actual menu content
    /// If not set, or the UiMenuBase is set to not use this container, levels are added independently to viewport
    /// Use a NamedSlot for this most of the time, it gives you the most layout flexibility.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
    UContentWidget* MenuContainer;

    /// Event raised when the stack is closed for any reason. If bWasCancel, the menu stack was closed because the
    /// last item was cancelled.
    UPROPERTY(BlueprintAssignable)
    FOnMenuStackClosed OnClosed;

    /// Push a new menu level by class. This will instantiate the new menu, display it, and inform the previous menu that it's
    /// been superceded. Use the returned instance if you want to cache it
    UFUNCTION(BlueprintCallable)
    UMenuBase* PushMenuByClass(TSubclassOf<UMenuBase> MenuClass);

    /// Push a new menu level by instance on to the stack. This will display the new menu and inform the previous menu that it's
    /// been superceded, which will most likely mean it will be hidden (but will retain its state)
    UFUNCTION(BlueprintCallable)
    void PushMenuByObject(UMenuBase* NewMenu);

    /// Pop the top level of the menu stack. This *destroys* the top level menu, meaning it will lose all of its state.
    /// You won't need to call this manually most of the time, because calling Close() on the MenuBase will do it.
    UFUNCTION(BlueprintCallable)
    void PopMenu(bool bWasCancel);

    /// Get the number of active levels in the menu
    UFUNCTION(BlueprintCallable)
    int Count() const { return Menus.Num(); }

    /// Close the entire stack at once. This does not give any of the menus chance to do anything before close, so if you
    /// want them to do that, use PopMenu() until Count() == 0 instead
    UFUNCTION(BlueprintCallable)
    void CloseAll(bool bWasCancel);


    virtual void SetFocusProperly_Implementation() override;
    void PopMenuIfTop(UMenuBase* UiMenuBase, bool bWasCancel);
};
