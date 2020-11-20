#pragma once
#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
enum class EInputMode : uint8
{
    Mouse,
    Keyboard,
    Gamepad,
    Unknown
};

UENUM(BlueprintType)
enum class EInputModeChange : uint8
{
    DoNotChange UMETA(DisplayName="No Change"),
    Restore UMETA(DisplayName="Restore To Previous"),
    UIOnly UMETA(DisplayName="UI Only"),
    GameAndUI UMETA(DisplayName="Game And UI"),
    GameOnly UMETA(DisplayName="Game Only")
};
    
UENUM(BlueprintType)
enum class EMousePointerVisibilityChange : uint8
{
    DoNotChange UMETA(DisplayName="No Change"),
    Restore UMETA(DisplayName="Restore To Previous"),
    Visible UMETA(DisplayName="Pointer Visible"),
    Hidden UMETA(DisplayName="Pointer Hidden")
};

UENUM(BlueprintType)
enum class EGamePauseChange : uint8
{
    DoNotChange UMETA(DisplayName="No Change"),
    Restore UMETA(DisplayName="Restore To Previous"),
    Paused UMETA(DisplayName="Pause Game"),
    Unpaused UMETA(DisplayName="Unpause Game")
};

UENUM(BlueprintType)
enum class EInputBindingType : uint8
{
    /// A button action, will be looked up based on input mappings
    Action = 0,
    /// An axis action, will be looked up based on input mappings
    Axis = 1,
    /// A manually specified FKey (which can be key, button, axis)
    Key = 2
};

