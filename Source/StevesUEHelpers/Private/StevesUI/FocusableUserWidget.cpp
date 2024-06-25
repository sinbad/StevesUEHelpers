#include "StevesUI/FocusableUserWidget.h"

#include "StevesUEHelpers.h"

void UFocusableUserWidget::SetFocusProperly_Implementation()
{
    // Default is to call normal
    SetFocus();
}


bool UFocusableUserWidget::IsRequestingFocus_Implementation() const
{
    // Subclasses can override this
    return bEnableAutomaticFocus;    
}

bool UFocusableUserWidget::TakeFocusIfDesired_Implementation()
{
    auto GS = GetStevesGameSubsystem(GetWorld());
    if (IsRequestingFocus() &&
        GS && (GS->GetLastInputModeUsed() != EInputMode::Gamepad || GS->GetLastInputModeUsed() != EInputMode::Keyboard))
    {
        SetFocusProperly();
        return true;
    }
    return false;
}

FReply UFocusableUserWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
    FReply Reply = Super::NativeOnFocusReceived(InGeometry, InFocusEvent);

    if (!Reply.IsEventHandled())
    {
        SetFocusProperly();
        Reply.Handled();
    }

    return Reply;
}

void UFocusableUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (bEnableAutomaticFocus)
    {
        auto GS = GetStevesGameSubsystem(GetWorld());
        if (GS)
            GS->GetFocusSystem()->FocusableWidgetConstructed(this);               
    }
}

void UFocusableUserWidget::NativeDestruct()
{
    Super::NativeDestruct();

    if (bEnableAutomaticFocus)
    {
        auto GS = GetStevesGameSubsystem(GetWorld());
        if (GS)
            GS->GetFocusSystem()->FocusableWidgetDestructed(this);               
    }

}
