#pragma once

#include "CoreMinimal.h"

#include "MenuStack.h"
#include "FocusablePanel.h"
#include "Blueprint/UserWidget.h"

#include "MenuBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMenuClosed, UMenuBase*, Menu, bool, bWasCancelled);

/// This class is a type of focusable panel designed for menus or other dialogs.
/// It can be added to a UMenuStack to put it in context of a larger navigable group,
/// and if so represents one level in the chain of an assumed modal stack. Use UMenuStack::PushMenuByClass/Object
/// to add an entry of this type to the stack
/// If you use this class standalone instead without a stack, then you must call Open() on this instance to
/// make it add itself to the viewport.
UCLASS(Abstract, BlueprintType)
class STEVESUEHELPERS_API UMenuBase : public UFocusablePanel
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable)
    FOnMenuClosed OnClosed;
    
protected:
    UPROPERTY(BlueprintReadOnly)
    TWeakObjectPtr<UMenuStack> ParentStack;

    /// Whether this menu should request focus when it is displayed
    /// The widget which is focussed will either be the InitialFocusWidget on newly displayed, or
    /// the previously selected widget if regaining focus
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
    bool bRequestFocus = true;

    /// Set this property to true if you want this menu to embed itself in the parent UMenuStack's MenuContainer
    /// If false, this Menu will be added to the viewport independently and can float over other menus in the stack
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Layout")
    bool bEmbedInParentContainer = true;

    /// Whether to hide this menu when it's superceded by another in the stack. This property is only relevant
    /// when bEmbedInParentContainer = false, since only one menu can be embedded at once.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Behavior")
    bool bHideWhenSuperceded = true;

    /// How this menu should set the input mode when it becomes the top of the stack
    /// This can be useful if your menus have variable input settings between levels in the stack
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Behavior")
    EInputModeChange InputModeSetting = EInputModeChange::DoNotChange;

    /// How this menu should set the mouse pointer visibility when it becomes the top of the stack
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Behavior")
    EMousePointerVisibilityChange MousePointerVisibility = EMousePointerVisibilityChange::DoNotChange;

    /// How this menu should set the game pause state when it becomes top of the stack
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Behavior")
    EGamePauseChange GamePauseSetting = EGamePauseChange::DoNotChange;

    virtual void EmbedInParent();

public:
    
    /**
     * @brief Open this menu. You should only call this if you're NOT using this in a UMenuStack, because the stack will
     * call it for you when you add this menu to it

     * @param bIsRegainedFocus Set this to true if the reason this menu is opening is that it regained focus in a stack
     */
    UFUNCTION(BlueprintCallable)
    void Open(bool bIsRegainedFocus = false); 
    /**
     * @brief Close this menu.
     * @param bWasCancel Set this to true if the reason for closure was a cancellation action
     */
    UFUNCTION(BlueprintCallable)
    void Close(bool bWasCancel);

    TWeakObjectPtr<UMenuStack> GetParentStack() const { return ParentStack; }
    virtual bool IsRequestingFocus_Implementation() const override { return bRequestFocus; }

    void AddedToStack(UMenuStack* Parent);
    void RemovedFromStack(UMenuStack* Parent);
    void SupercededInStack();
    void RegainedFocusInStack();
    void InputModeChanged(EInputMode OldMode, EInputMode NewMode);
};
