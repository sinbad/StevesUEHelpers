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

