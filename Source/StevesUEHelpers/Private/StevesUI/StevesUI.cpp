// Copyright Steve Streeting 2020 onwards
// Released under the MIT license
#include "StevesUI.h"



#include "StevesUI/FocusableUserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Components/Widget.h"

DEFINE_LOG_CATEGORY(LogStevesUI);

UWidget* FindWidgetFromSlate(SWidget* SW, UWidget* Parent)
{
    if (!Parent)
        return nullptr;

    if (Parent->GetCachedWidget().Get() == SW)
        return Parent;

    auto PW = Cast<UPanelWidget>(Parent);
    if (PW)
    {
        for (int i = 0; i < PW->GetChildrenCount(); ++i)
        {
            const auto Found = FindWidgetFromSlate(SW, PW->GetChildAt(i));
            if (Found)
                return Found;
        }
    }
    else
    {
        // User widgets aren't panels but can have their own tree
        auto UW = Cast<UUserWidget>(Parent);
        if (UW)
        {
            return FindWidgetFromSlate(SW, UW->WidgetTree->RootWidget);
        }
        
    }
    return nullptr;
}

void SetWidgetFocusProperly(UWidget* Widget)
{
    auto FW = Cast<UFocusableUserWidget>(Widget);
    if (FW)
        FW->SetFocusProperly();
    else
        Widget->SetFocus();
    
}

const FKey* GetPreferedKeyMapping(const TArray<FKey>& AllKeys,
    EInputImageDevicePreference DevicePreference,
    EInputMode LastInputDevice,
    EInputMode LastButtonInputDevice,
    EInputMode LastAxisInputDevice)
{
    // Same as GetPreferedActionOrAxisMapping, just with key directly
    const FKey* MouseMapping = nullptr;
    const FKey* KeyboardMapping = nullptr;
    const FKey* GamepadMapping = nullptr;
    for (const FKey& Key : AllKeys)
    {
        // notice how we take the LAST one in the list as the final version
        // this is because UInputSettings::GetActionMappingByName *reverses* the mapping list from Project Settings
        if (Key.IsGamepadKey())
        {
            GamepadMapping = &Key;
        }
        else if (Key.IsMouseButton()) // registers true for mouse axes too
        {
            MouseMapping = &Key;
        }
        else
        {
            KeyboardMapping = &Key;
        }
    }

    const FKey* Preferred = nullptr;
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

void GetPreferedKeyMappings(TArray<FKey>& OutPreferredKeys,
	const TArray<FKey>& InKeys,
	EInputImageDevicePreference DevicePreference,
	EInputMode LastInputDevice,
	EInputMode LastButtonInputDevice,
	EInputMode LastAxisInputDevice)
{
	OutPreferredKeys.Reset();
	
	bool bHasGamepadInput = false;
	bool bHasKeyboardInput = false;
	bool bHasMouseInput = false;
	
    for (const FKey& Key : InKeys)
    {
    	// Add all of them, then prune later
    	// Except don't bother to add gamepad options unless gamepad is in use
    	if (!Key.IsGamepadKey() || LastInputDevice == EInputMode::Gamepad)
    		OutPreferredKeys.Add(Key);
    	
    	if (Key.IsGamepadKey())
		{
        	bHasGamepadInput = true;
        }
    	else if (Key.IsMouseButton())
    	{
    		bHasMouseInput = true;
    	}
	    else
	    {
		    bHasKeyboardInput = true;
	    }

    }

	// Now prune inputs based on preference
	// We do this so that even if we one prefer input mode, if there isn't any mapping of that type
	// we fall back on something from mouse / keyboard

	if (LastInputDevice == EInputMode::Gamepad && bHasGamepadInput)
	{
		// Prune any non-gamepad inputs since we have one
		OutPreferredKeys.RemoveAll([](FKey& Key)
		{
			return !Key.IsGamepadKey(); 
		});

	}
	else
	{
		bool bPreferMouse = false;
		switch (DevicePreference)
		{
			// Auto should be pre-converted to another
		case EInputImageDevicePreference::Auto:
			UE_LOG(LogStevesUI, Error, TEXT("Device Preference should have been converted before this call"))
			break;
		case EInputImageDevicePreference::Gamepad_Keyboard_Mouse:
			break;
		case EInputImageDevicePreference::Gamepad_Mouse_Keyboard:
			bPreferMouse = true;
			break;

		case EInputImageDevicePreference::Gamepad_Keyboard_Mouse_Button:
			// Use the latest button press
			bPreferMouse = LastButtonInputDevice == EInputMode::Mouse;
			break;
		case EInputImageDevicePreference::Gamepad_Keyboard_Mouse_Axis:
			// Use the latest axis move
			bPreferMouse = LastAxisInputDevice == EInputMode::Mouse;
			break;
		default:
			break;
		}
		
		// If there are both keyboard and mouse inputs, prune one based on preference
		if (bHasMouseInput && bHasKeyboardInput)
		{
			if (bPreferMouse)
			{
				// Prune keyboard
				OutPreferredKeys.RemoveAll([](FKey& Key)
				{
					return !Key.IsMouseButton(); // Also axes
				});
			}
			else
			{
				// Prune mouse
				OutPreferredKeys.RemoveAll([](FKey& Key)
				{
					return Key.IsMouseButton(); // Also axes
				});
			}
		}
		
		
	}

}
