#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabSelected,int,TabIndex);

/**
 * Tab button - widget that is instantiated on a tab list
 */
UCLASS()
class STEVESUEHELPERS_API UTabButton : public UUserWidget
{
	GENERATED_BODY()


public:
	
	///
	
	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FOnTabSelected OnTabSelected;
	
	UPROPERTY(BlueprintReadOnly, Category="TabButton")
	int TabIndex;

	UPROPERTY(BlueprintReadOnly, Category="TabButton")
	bool bTabSelected=false;

	void Initialise(int InTabIndex,const FText& InLabel);

	UFUNCTION(BlueprintNativeEvent, Category="TabButton")
	void SetTabLabel(const FText& InLabel);
	
	UFUNCTION(BlueprintCallable, Category="TabButton")
	void SetTabSelected(bool bSelected);
	
	UFUNCTION(BlueprintCallable, Category="TabButton")
	void NotifyTabSelected();

	UFUNCTION(BlueprintNativeEvent, Category="TabButton")
	void RefreshForSelectionChanged();
	
};
