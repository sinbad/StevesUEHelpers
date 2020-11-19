#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMenuSystem, Log, All)

class FMenuSystem
{
protected:
    TArray<TWeakObjectPtr<class UMenuStack>> ActiveMenuStacks;

    TWeakObjectPtr<UMenuStack> GetHighestFocusPriority();
public:
    void MenuStackOpened(UMenuStack* Stack);
    void MenuStackClosed(UMenuStack* Stack);
    
};
