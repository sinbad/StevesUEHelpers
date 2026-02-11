// Copyright Steve Streeting 2020 onwards
// Released under the MIT license
#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFocusSystem, Log, All)

class FFocusSystem
{
protected:
    TArray<TWeakObjectPtr<class UFocusableUserWidget>> ActiveAutoFocusWidgets;

    TWeakObjectPtr<UFocusableUserWidget> GetHighestFocusPriority();
public:
    void FocusableWidgetConstructed(UFocusableUserWidget* Widget);
    void FocusableWidgetDestructed(UFocusableUserWidget* Widget);
    
};
