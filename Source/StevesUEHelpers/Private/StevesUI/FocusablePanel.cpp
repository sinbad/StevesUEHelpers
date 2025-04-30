#include "StevesUI/FocusablePanel.h"


#include "StevesUI.h"
#include "Blueprint/WidgetTree.h"
#include "Framework/Application/SlateApplication.h"

void UFocusablePanel::NativeConstruct()
{
    Super::NativeConstruct();

    // Find focus widget
    
    InitialFocusWidget = GetInitialFocusWidget();
#if WITH_EDITOR
    if (!InitialFocusWidget.IsValid())
    {
        UE_LOG(LogStevesUI, Error, TEXT("Initial focus widget `%s` not found on %s, focus will be lost"), *InitialFocusWidgetName.ToString(), *GetName())
    }
#endif

}



void UFocusablePanel::NativeDestruct()
{
    Super::NativeDestruct();

    InitialFocusWidget.Reset();
}

bool UFocusablePanel::SetFocusToInitialWidget()
{
    if (!InitialFocusWidget.IsValid())
    {
        // if not set, attempt to get
        InitialFocusWidget = GetInitialFocusWidget();
    }
    if (InitialFocusWidget.IsValid())
    {
        SetWidgetFocusProperly(InitialFocusWidget.Get());
        return true;
    }
    return false;
}

UWidget* UFocusablePanel::GetInitialFocusWidget_Implementation()
{
    if (!InitialFocusWidgetName.IsNone())
    {
        return WidgetTree->FindWidget(InitialFocusWidgetName);
    }
    return nullptr;
}


bool UFocusablePanel::RestorePreviousFocus() const
{
    if (PreviousFocusWidget.IsValid())
    {
        SetWidgetFocusProperly(PreviousFocusWidget.Get());
        return true;
    }
    return false;
}

bool UFocusablePanel::SavePreviousFocus()
{
    const auto SW = FSlateApplication::Get().GetUserFocusedWidget(0);
    if (SW)
    {
        PreviousFocusWidget = FindWidgetFromSlate(SW.Get(), this);
        return true;
    }
    else
    {
        PreviousFocusWidget.Reset();
        return false;
    }
}

void UFocusablePanel::SetFocusProperly_Implementation()
{
    if (!RestorePreviousFocus())
        SetFocusToInitialWidget();
}

