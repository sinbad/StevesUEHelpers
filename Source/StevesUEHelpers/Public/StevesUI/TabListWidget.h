#pragma once

#include "CoreMinimal.h"
#include "StevesUI/FocusableInputInterceptorUserWidget.h"
#include "TabListWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnContentWidgetChanged,UWidget*,PreviousContentWidget,UWidget*,NewContentWidget);

class UWidgetSwitcher;
class UTabButton;
/** Information about a registered tab in the tab list */
USTRUCT()
struct FTabListRegisteredTabInfo
{
	GENERATED_BODY()

public:
	/** The index of this tab */
	UPROPERTY()
	int32 TabIndex;
	
	/** The button for this tab */
	UPROPERTY()
	TObjectPtr<UTabButton> TabButton;

	/** The content to be activated. */
	UPROPERTY()
	TObjectPtr<UWidget> ContentInstance;

	FTabListRegisteredTabInfo()
		: TabIndex(INDEX_NONE)
		, TabButton(nullptr)
		, ContentInstance(nullptr)
	{}
};


/**
 * Tab list widget - provides a tab selection bar that can control a widget switcher
 */
UCLASS()
class STEVESUEHELPERS_API UTabListWidget : public UFocusableInputInterceptorUserWidget
{
	GENERATED_BODY()
public:


	/// Event called when tab content has changed
	UPROPERTY(BlueprintCallable,BlueprintAssignable)
	FOnContentWidgetChanged OnContentWidgetChanged;
	
	/// Input keys which navigate to the previous tab (left)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TArray<FKey> PreviousTabKeys = TArray<FKey> { EKeys::Gamepad_LeftShoulder, EKeys::Q };
	
	/// Input keys which navigate to the next tab (right)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TArray<FKey> NextTabKeys = TArray<FKey> { EKeys::Gamepad_RightShoulder, EKeys::E };

	/// Map of tabs, buttons, content
	TMap<FName,FTabListRegisteredTabInfo> RegisteredTabs;

	// Where the tab buttons will be added
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UPanelWidget> TabButtonContainer;
	
	UPROPERTY(BlueprintReadOnly)
	int SelectedTabIndex=-1;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FMargin TabButtonPadding=FMargin(10,0);

	UPROPERTY(BlueprintReadOnly)
	UWidgetSwitcher* TargetWidgetSwitcher;
	
	/**
	 * Registers tab content with this tab widget
	 * @param TabID The ID to use for this widget
	 * @param ButtonWidgetType The class of button to use
	 * @param ContentWidget The widget to activate when pressed
	 * @return True If successful
	 */
	UFUNCTION(BlueprintCallable, Category = TabList)
	bool RegisterTab(FName TabID, TSubclassOf<UTabButton> ButtonWidgetType,FText Label, UWidget* ContentWidget);

	/**
	 * Called when a given tab button has been pressed
	 * @param TabIndex 
	 */
	UFUNCTION()
	void TabPressed(int TabIndex);

	/**
	 * Assign a widget switcher to be driven by this tab selection.
	 * @param WidgetSwitcher 
	 */
	UFUNCTION(BlueprintCallable)
	void SetTargetWidgetSwitcher(UWidgetSwitcher* WidgetSwitcher);

	UFUNCTION(BlueprintCallable)
	void IncrementTabIndex(int Amount);
	
	UFUNCTION(BlueprintCallable)
	void SelectTab(int TabIndex);

	virtual bool HandleKeyDownEvent(const FKeyEvent& InKeyEvent) override;

private:
	bool GetTabDataForIndex(int Index,FTabListRegisteredTabInfo& TabInfo);
};
