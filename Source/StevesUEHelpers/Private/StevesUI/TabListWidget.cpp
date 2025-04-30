#include "StevesUI/TabListWidget.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "StevesUI/TabButton.h"
#include "StevesUI/FocusableButton.h"
#include "StevesUI/MenuBase.h"
#include "Widgets/SPanel.h"


void UTabListWidget::SetTargetWidgetSwitcher(UWidgetSwitcher* WidgetSwitcher)
{
	TargetWidgetSwitcher=WidgetSwitcher;
}

bool UTabListWidget::RegisterTab(const FName TabID, const TSubclassOf<UTabButton> ButtonWidgetType,FText Label,
	UWidget* ContentWidget)
{

	if (RegisteredTabs.Contains(TabID))
	{
		UE_LOG(LogTemp,Error,TEXT("UTabListWidget::RegisterTab Already Registered ID %s"),*TabID.ToString());
		return false;
	}
	if (TabButtonContainer==nullptr)
	{
		UE_LOG(LogTemp,Error,TEXT("UTabListWidget::RegisterTab TabButtonContainer is not set"));
		return false;
	}
	FName const ButtonName( *FString::Printf(TEXT("TabButton_%s"),*TabID.ToString()));
	UTabButton* const NewTabButton = Cast<UTabButton>(CreateWidget(this,ButtonWidgetType,ButtonName));
	const int CurrentTabCount = RegisteredTabs.Num();
	const int NewTabIndex = CurrentTabCount;
	
	FTabListRegisteredTabInfo TabInfo;
	TabInfo.TabIndex = NewTabIndex;
	TabInfo.TabButton = NewTabButton;
	TabInfo.ContentInstance = ContentWidget;
	RegisteredTabs.Add(TabID, TabInfo);
	UPanelSlot* ButtonPanelSlot=TabButtonContainer->AddChild(NewTabButton);
	UHorizontalBoxSlot* HorizontalBoxSlot=Cast<UHorizontalBoxSlot>(ButtonPanelSlot);
	if (HorizontalBoxSlot)
	{
		HorizontalBoxSlot->SetPadding(TabButtonPadding);
	}
	
	NewTabButton->OnTabSelected.AddUniqueDynamic(this,&UTabListWidget::TabPressed);
	NewTabButton->TabIndex=NewTabIndex;
	NewTabButton->Initialise(NewTabIndex,Label);

	// If first tab added, select this
	if (CurrentTabCount==0)
	{
		SelectTab(0);
	}
	return true;
}

void UTabListWidget::TabPressed(int TabIndex)
{
	SelectTab(TabIndex);
}


bool UTabListWidget::GetTabDataForIndex(const int Index,FTabListRegisteredTabInfo& TabInfo)
{
	for (const auto& InfoPair : RegisteredTabs)
	{
		if (InfoPair.Value.TabIndex==Index)
		{
			TabInfo=InfoPair.Value;
			return true;
		}
	}
	return false;
}

void UTabListWidget::IncrementTabIndex(int Amount)
{
	int NewTabIndex=SelectedTabIndex+Amount;
	while (NewTabIndex<0) NewTabIndex+=RegisteredTabs.Num();
	NewTabIndex%=RegisteredTabs.Num();
	SelectTab(NewTabIndex);
	
}
void UTabListWidget::SelectTab(int TabIndex)
{
	if (TabIndex==SelectedTabIndex) return; // No Change

	// If a tab button has previously been selected, deselect
	if (SelectedTabIndex!=-1)
	{
		FTabListRegisteredTabInfo PreviousTabInfo;
		if (GetTabDataForIndex(SelectedTabIndex,PreviousTabInfo))
		{
			PreviousTabInfo.TabButton->SetTabSelected(false);
		} else
		{
			UE_LOG(LogTemp,Warning,TEXT("Couldn't find button for previously index %i"),SelectedTabIndex);
		}
	}
	SelectedTabIndex=TabIndex;
	FTabListRegisteredTabInfo CurrentTabInfo;
	if (GetTabDataForIndex(SelectedTabIndex,CurrentTabInfo))
	{
		CurrentTabInfo.TabButton->SetTabSelected(true);
	}

	if (!TargetWidgetSwitcher)
	{
		UE_LOG(LogTemp,Warning,TEXT("TargetWidgetSwitcher not set"));
		return;
	}
	if (!CurrentTabInfo.ContentInstance)
	{
		UE_LOG(LogTemp,Warning,TEXT("Content not set for tab index %i"),SelectedTabIndex);
		return;
	}
	
	// If there is a current content widget, cache the current focus so that it can be restored when moving back to this tab
	UWidget* PreviousContentWidget=TargetWidgetSwitcher->GetActiveWidget();
	if (PreviousContentWidget)
	{
		if (UFocusablePanel* TargetFocusableWidget=Cast<UFocusablePanel>(PreviousContentWidget))
		{
			TargetFocusableWidget->SavePreviousFocus();
		}
	}
	
	TargetWidgetSwitcher->SetActiveWidget(CurrentTabInfo.ContentInstance);
	
	// If this is an instance of UFocusableUserWidget, notify to activate default focus
	if (UFocusableUserWidget* TargetFocusableWidget=Cast<UFocusableUserWidget>(CurrentTabInfo.ContentInstance))
	{
		UE_LOG(LogTemp,Warning,TEXT("Focusable widget found for widget %s"),*CurrentTabInfo.ContentInstance->GetName());
		TargetFocusableWidget->SetFocusProperly();
	}else
	{
		UE_LOG(LogTemp,Warning,TEXT("Widget not focusable widget %s"),*CurrentTabInfo.ContentInstance->GetName());
	}
	// Notify
	OnContentWidgetChanged.Broadcast(PreviousContentWidget,CurrentTabInfo.ContentInstance);
}

bool UTabListWidget::HandleKeyDownEvent(const FKeyEvent& InKeyEvent)
{
	Super::HandleKeyDownEvent(InKeyEvent);
	
	const FKey Key = InKeyEvent.GetKey();
	if (PreviousTabKeys.Contains(Key))
	{
		IncrementTabIndex(-1);
		return true;
	}
	if (NextTabKeys.Contains(Key))
	{
		IncrementTabIndex(1);
		return true;
	}
	return false;
}


