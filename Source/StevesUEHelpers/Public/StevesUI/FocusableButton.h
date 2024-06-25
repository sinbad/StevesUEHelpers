#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "UObject/ObjectMacros.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SWidget.h"

#include "FocusableButton.generated.h"

class SFocusableButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonFocusReceivedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonFocusLostEvent);

/**
* This is a simple subclass of UButton to provide some missing features
*
*  * Focus events
*  * Focus style based on hover style
*  * Assign focus to self on hover to prevent double-highlighting
*/
UCLASS()
class STEVESUEHELPERS_API UFocusableButton : public UButton
{
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bUseHoverStyleWhenFocussed = true;

    UPROPERTY(BlueprintAssignable, Category="Button|Event")
    FOnButtonFocusReceivedEvent OnFocusReceived;

    UPROPERTY(BlueprintAssignable, Category="Button|Event")
    FOnButtonFocusLostEvent OnFocusLost;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bTakeFocusOnHover = true;

    // Simulate a button press
    UFUNCTION(BlueprintCallable)
    void SimulatePress();
    // Simulate a button release
    UFUNCTION(BlueprintCallable)
    void SimulateRelease();

protected:    
    FButtonStyle FocussedStyle;

    void SlateHandleFocusReceived();
    void SlateHandleFocusLost();

    void ApplyFocusStyle();
    void UndoFocusStyle();
    void SlateHandleHovered();
    void SlateHandleUnhovered();

    virtual TSharedRef<SWidget> RebuildWidget() override;

    
    /// Update the focussed style based on changes made to the default widget style.
    /// Call this if you make runtime changes to the base style of this button.
    /// Needed because we can't override SetStyle
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void RefreshFocussedStyle();
};