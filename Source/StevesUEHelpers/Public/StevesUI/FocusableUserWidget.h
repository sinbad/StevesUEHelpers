#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FocusableUserWidget.generated.h"

// Hacky intermediate type for UUserWidget so that we can have focusable child widgets via SetFocusProperly
UCLASS()
class STEVESUEHELPERS_API UFocusableUserWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /// UWidget::SetFocus is not virtual FFS. This does the same as SetFocus by default but can be overridden,
    /// e.g. to delegate focus to specific children
    UFUNCTION(BlueprintNativeEvent)
    void SetFocusProperly();
        
};
