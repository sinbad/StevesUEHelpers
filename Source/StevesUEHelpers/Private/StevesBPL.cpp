// Fill out your copyright notice in the Description page of Project Settings.


#include "StevesBPL.h"

#include "StevesUiHelpers.h"
#include "StevesUI/StevesUI.h"

void UStevesBPL::SetWidgetFocus(UWidget* Widget)
{
	SetWidgetFocusProperly(Widget);
}

void UStevesBPL::InsertChildWidgetAt(UPanelWidget* Parent, UWidget* Child, int AtIndex)
{
	StevesUiHelpers::InsertChildWidgetAt(Parent, Child, AtIndex);
}
