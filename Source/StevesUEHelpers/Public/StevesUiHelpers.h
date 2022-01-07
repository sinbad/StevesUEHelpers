#pragma once
#include "CoreMinimal.h"
#include "Components/Widget.h"

/// Helper Ui routines that UE4 is missing, all static
/// Exposed to BP in in StevesBPL
class STEVESUEHELPERS_API StevesUiHelpers
{
public:
	/**
	 * Insert a child widget at a specific index
	 * @param Parent The container widget
	 * @param Child The child widget to add
	 * @param AtIndex The index at which the new child should exist
	 */
	static void InsertChildWidgetAt(UPanelWidget* Parent, UWidget* Child, int AtIndex = 0);
};
