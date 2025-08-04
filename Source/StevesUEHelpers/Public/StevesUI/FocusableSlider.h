// 

#pragma once

#include "CoreMinimal.h"
#include "Components/Slider.h"
#include "FocusableSlider.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSliderHoveredEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSliderUnhoveredEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSliderFocusReceivedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSliderFocusLostEvent);

/**
 * A focusable version of the slider control, which highlights things like hover when the controller
 * has the focus on it.
 */
UCLASS()
class STEVESUEHELPERS_API UFocusableSlider : public USlider
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Focus")
	bool bUseHoverStyleWhenFocussed = true;

	UPROPERTY( BlueprintAssignable, Category = "Slider|Event" )
	FOnSliderHoveredEvent OnHovered;

	UPROPERTY( BlueprintAssignable, Category = "Slider|Event" )
	FOnSliderUnhoveredEvent OnUnhovered;

	UPROPERTY(BlueprintAssignable, Category="Slider|Event")
	FOnSliderFocusReceivedEvent OnFocusReceived;

	UPROPERTY(BlueprintAssignable, Category="Slider|Event")
	FOnSliderFocusLostEvent OnFocusLost;
    
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Focus")
	bool bTakeFocusOnHover = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Focus")
	bool bLoseFocusOnUnhover = true;

protected:    
	FSliderStyle FocussedStyle;

	void SlateHandleFocusReceived();
	void SlateHandleFocusLost();

	void ApplyFocusStyle();
	void UndoFocusStyle();
	void SlateHandleHovered();
	void SlateHandleUnhovered();

	void Unfocus() const;

	virtual TSharedRef<SWidget> RebuildWidget() override;
	
public:

	/// Update the focussed style based on changes made to the default widget style.
	/// Call this if you make runtime changes to the base style of this checkbox.
	/// Needed because we can't override SetStyle
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Focus")
	void RefreshFocussedStyle();
};
