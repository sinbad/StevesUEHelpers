#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"


#include "UiTheme.generated.h"

/// Custom asset to conveniently hold theme information for the UI
/// Currently only lightly used to provide simple access to button images, but I intend to use
/// this more extensively later
UCLASS(Blueprintable)
class STEVESUEHELPERS_API UUiTheme : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditDefaultsOnly)
    TSoftObjectPtr<UDataTable> KeyboardMouseImages;
    UPROPERTY(EditDefaultsOnly)
    TSoftObjectPtr<UDataTable> XboxControllerImages;
    
};
