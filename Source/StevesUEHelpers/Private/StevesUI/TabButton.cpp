#include "StevesUI/TabButton.h"

void UTabButton::Initialise(int InTabIndex,const FText& InLabel)
{
	TabIndex=InTabIndex;
	SetTabLabel(InLabel);
}

void UTabButton::SetTabLabel_Implementation(const FText& InLabel)
{
}

void UTabButton::SetTabSelected(bool bSelected)
{
	bTabSelected=bSelected;
	RefreshForSelectionChanged();
}

void UTabButton::RefreshForSelectionChanged_Implementation()
{
	
}

void UTabButton::NotifyTabSelected()
{
	OnTabSelected.Broadcast(TabIndex);
}
