#pragma once

#include "CoreMinimal.h"

#include "MenuStack.h"
#include "FocusablePanel.h"
#include "Blueprint/UserWidget.h"

#include "MenuBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMenuClosed, UMenuBase*, Menu, bool, bWasCancelled);
/// This class is one element of the UUiMenuStack and represents one level in the chain of
/// an assumed modal stack. It is responsible for implementing how it gets added
/// to the viewport or parent container, and removed again, and what it does when receiving
/// focus.
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequestFocus = true;

    /// Set this property to true if you want this menu to embed itself in the parent UUiMenuStack's MenuContainer
    /// If false, this Menu will be added to the viewport independently and can float over other menus in the stack
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEmbedInParentContainer = true;

    /// Whether to hide this menu when it's superceded by another in the stack. This property is only relevant
    /// when bEmbedInParentContainer = false, since only one menu can be embedded at once.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHideWhenSuperceded = true;

    void EmbedInParent();

public:
    /// Close this menu level
    UFUNCTION(BlueprintCallable)
    void Close(bool bWasCancel);

    TWeakObjectPtr<UMenuStack> GetParentStack() const { return ParentStack; };

    void AddedToStack(UMenuStack* Parent);
    void RemovedFromStack(UMenuStack* Parent);
    void SupercededInStack();
    void RegainedFocusInStack();
    void InputModeChanged(EInputMode OldMode, EInputMode NewMode);
};
