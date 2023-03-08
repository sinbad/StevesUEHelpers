#pragma once

#include "CoreMinimal.h"
#include "StevesHelperCommon.h"

class UWidget;
class SWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogStevesUI, Warning, Warning)


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

template <typename T>
const T* GetPreferedActionOrAxisMapping(const TArray<T>& AllMappings, const FName& Name,
                                                   EInputImageDevicePreference DevicePreference,
                                                   EInputMode LastInputDevice,
                                                   EInputMode LastButtonInputDevice,
                                                   EInputMode LastAxisInputDevice)
{
    const T* MouseMapping = nullptr;
    const T* KeyboardMapping = nullptr;
    const T* GamepadMapping = nullptr;
    for (const T& ActionMap : AllMappings)
    {
        // notice how we take the LAST one in the list as the final version
        // this is because UInputSettings::GetActionMappingByName *reverses* the mapping list from Project Settings
        if (ActionMap.Key.IsGamepadKey())
        {
            GamepadMapping = &ActionMap;
        }
        else if (ActionMap.Key.IsMouseButton()) // registers true for mouse axes too
        {
            MouseMapping = &ActionMap;
        }
        else
        {
            KeyboardMapping = &ActionMap;
        }
    }

    const T* Preferred = nullptr;
    if (GamepadMapping && LastInputDevice == EInputMode::Gamepad)
    {
        // Always prefer gamepad if used last
        Preferred = GamepadMapping;
    }
    else
    {
        switch (DevicePreference)
        {
        // Auto should be pre-converted to another
        case EInputImageDevicePreference::Auto:
            UE_LOG(LogStevesUI, Error, TEXT("Device Preference should have been converted before this call"))
            break;
        case EInputImageDevicePreference::Gamepad_Keyboard_Mouse:
            Preferred = KeyboardMapping ? KeyboardMapping : MouseMapping;
            break;
        case EInputImageDevicePreference::Gamepad_Mouse_Keyboard:
            Preferred = MouseMapping ? MouseMapping : KeyboardMapping;
            break;

        case EInputImageDevicePreference::Gamepad_Keyboard_Mouse_Button:
            // Use the latest button press
            Preferred = (MouseMapping && (LastButtonInputDevice == EInputMode::Mouse || !KeyboardMapping)) ? MouseMapping : KeyboardMapping;
            break;
        case EInputImageDevicePreference::Gamepad_Keyboard_Mouse_Axis:
            // Use the latest button press
            Preferred = (MouseMapping && (LastAxisInputDevice == EInputMode::Mouse || !KeyboardMapping)) ? MouseMapping : KeyboardMapping;
            break;
        default:
            break;
        }
    }
    return Preferred;
}


const FKey* GetPreferedKeyMapping(const TArray<FKey>& AllKeys,
                                  EInputImageDevicePreference DevicePreference,
                                  EInputMode LastInputDevice,
                                  EInputMode LastButtonInputDevice,
                                  EInputMode LastAxisInputDevice);
