#pragma once

#include "CoreMinimal.h"

#include "InputCoreTypes.h"
#include "Engine/DataTable.h"
#include "PaperSprite.h"
#include "KeySprite.generated.h"

/// Struct for the rows of a DataTable which will hold the mapping from an FKey to a Paper Sprite which represents it
/// Used for on-screen prompts
USTRUCT(BlueprintType)
struct STEVESUEHELPERS_API FKeySprite : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

    // Import a DataTable using this struct by creating a CSV file like this:
    // 
    // Name,Key,Sprite
    // 1,Enter,"PaperSprite'/Game/Textures/UI/Frames/Keyboard_Black_Enter'"
    // 2,SpaceBar,"PaperSprite'/Game/Textures/UI/Frames/Keyboard_Black_Space'"
    //
    // Key is just the latter part of EKeys::Name
    // Sprite is the path to the Paper2D sprite (most likely from a shared sprite sheet)
    
public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FKey Key;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UPaperSprite* Sprite = nullptr;
};