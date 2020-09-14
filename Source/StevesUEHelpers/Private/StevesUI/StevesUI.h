#pragma once

#include "CoreMinimal.h"

class UWidget;
class SWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogCustomUI, Warning, Warning)


/**
 * @brief Tries to locate a UMG widget which is using the speficied Slate widget as its native implementation
 * @param SW Slate widget
 * @param Parent Parent widget under which the UMG widget should be found
 * @return The UMG widget if found, otherwise nullptr
 */
UWidget* FindWidgetFromSlate(SWidget* SW, UWidget* Parent);

/**
 * @brief Set the focus to a given widget "properly", which means that if this is a widget derived
 * from UFocusableWidget, it calls SetFocusProperly on it which allows a customised implementation.
 * This is done because SetFocus() is not virtual and cannot be changed, and bIsFocusable seems to get
 * reset by Slate code even if I try to set it and then override Native methods.
 * @param Widget A UWidget
 */
void SetWidgetFocusProperly(UWidget* Widget);
