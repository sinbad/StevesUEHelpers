// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CheckBox.h"
#include "FocusableCheckBox.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCheckBoxHoveredEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCheckBoxUnhoveredEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCheckBoxFocusReceivedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCheckBoxFocusLostEvent);
/**
* This is a simple subclass of UCheckBox to provide some missing features
*
*  * Focus events
*  * Hover events
*  * Focus style based on hover style
*  * Assign focus to self on hover to prevent double-highlighting
*/
UCLASS()
class STEVESUEHELPERS_API UFocusableCheckBox : public UCheckBox
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bUseHoverStyleWhenFocussed = true;

	UPROPERTY( BlueprintAssignable, Category = "CheckBox|Event" )
	FOnCheckBoxHoveredEvent OnHovered;

	UPROPERTY( BlueprintAssignable, Category = "CheckBox|Event" )
	FOnCheckBoxUnhoveredEvent OnUnhovered;

	
	UPROPERTY(BlueprintAssignable, Category="CheckBox|Event")
	FOnCheckBoxFocusReceivedEvent OnFocusReceived;

	UPROPERTY(BlueprintAssignable, Category="CheckBox|Event")
	FOnCheckBoxFocusLostEvent OnFocusLost;
    
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bTakeFocusOnHover = true;
protected:    
	FCheckBoxStyle FocussedStyle;

	void SlateHandleFocusReceived();
	void SlateHandleFocusLost();

	void ApplyFocusStyle();
	void UndoFocusStyle();
	void SlateHandleHovered();
	void SlateHandleUnhovered();

	virtual TSharedRef<SWidget> RebuildWidget() override;

public:

	/// Update the focussed style based on changes made to the default widget style.
	/// Call this if you make runtime changes to the base style of this checkbox.
	/// Needed because we can't override SetStyle
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RefreshFocussedStyle();
	
	
};
