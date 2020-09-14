#include "StevesUI.h"



#include "StevesUI/FocusableUserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Components/Widget.h"

DEFINE_LOG_CATEGORY(LogCustomUI);

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
