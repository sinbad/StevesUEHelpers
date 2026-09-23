// Copyright Steve Streeting 2020 onwards
// Released under the MIT license
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
	 * @returns The slot the child was inserted at
	 */
	static UPanelSlot* InsertChildWidgetAt(UPanelWidget* Parent, UWidget* Child, int AtIndex = 0);
	/**
	 * Utility function to find an input widget based on an input direction. Unlike regular UI navigation,
	 * this supports a full 2D direction and not juse Left/Right/Up/Down
	 * @param FromWidget The widget to start from
	 * @param Direction The input direction
	 * @param InParent The parent container to search for widgets. This is because we can't use the internal Slate
	 * hit grid (it's inaccessible) so we have to check widgets manually
	 * @param AngleDegrees The angle to search for hits
	 * @return 
	 */
	static UWidget* FindClosestWidgetInInputDirection(UWidget* FromWidget,
	                                                  const FVector2D& Direction,
	                                                  UPanelWidget* InParent,
	                                                  float AngleDegrees = 30);
};
