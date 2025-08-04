// 


#include "StevesUI/FocusableSlider.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/LocalPlayer.h"

#include "SFocusableSlider.h"

TSharedRef<SWidget> UFocusableSlider::RebuildWidget()
{
	MySlider = SNew(SFocusableSlider)
		.Style(&GetWidgetStyle())
		.OnMouseCaptureBegin(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnMouseCaptureBegin))
		.OnMouseCaptureEnd(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnMouseCaptureEnd))
		.OnControllerCaptureBegin(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnControllerCaptureBegin))
		.OnControllerCaptureEnd(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnControllerCaptureEnd))
		.OnValueChanged(BIND_UOBJECT_DELEGATE(FOnFloatValueChanged, HandleOnValueChanged))
		// Our new events
	    .OnHovered(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleHovered))
		.OnUnhovered(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleUnhovered))
		.OnFocusReceived(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleFocusReceived))
		.OnFocusLost(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleFocusLost))
	;

	RefreshFocussedStyle();

    
	return MySlider.ToSharedRef();
   
}

void UFocusableSlider::RefreshFocussedStyle_Implementation()
{
    // Copy Widget style but make normal same as hovered    
    FocussedStyle = GetWidgetStyle();
    FocussedStyle.NormalBarImage = FocussedStyle.HoveredBarImage;
    FocussedStyle.NormalThumbImage = FocussedStyle.HoveredThumbImage;
}

void UFocusableSlider::SlateHandleFocusReceived()
{
    ApplyFocusStyle();
    OnFocusReceived.Broadcast();
}

void UFocusableSlider::SlateHandleFocusLost()
{
    UndoFocusStyle();
    OnFocusLost.Broadcast();
}


void UFocusableSlider::ApplyFocusStyle()
{
    if (MySlider.IsValid() && bUseHoverStyleWhenFocussed)
    {
        MySlider->SetStyle(&FocussedStyle);
    }
}

void UFocusableSlider::UndoFocusStyle()
{
    if (MySlider.IsValid())
    {
        MySlider->SetStyle(&GetWidgetStyle());
    }
}

void UFocusableSlider::SlateHandleHovered()
{
    if (bTakeFocusOnHover)
    {
        SetFocus();
    }
    OnHovered.Broadcast();
}

void UFocusableSlider::SlateHandleUnhovered()
{
	if (bLoseFocusOnUnhover)
	{
		Unfocus();
	}
	OnUnhovered.Broadcast();
}

void UFocusableSlider::Unfocus() const
{
	APlayerController* OwningPlayer = GetOwningPlayer();
	if (OwningPlayer == nullptr || !OwningPlayer->IsLocalController() || OwningPlayer->Player ==
		nullptr)
	{
		return;
	}
	if (ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer())
	{
		TOptional<int32> UserIndex = FSlateApplication::Get().GetUserIndexForController(
			LocalPlayer->GetControllerId());
		if (UserIndex.IsSet())
		{
			TSharedPtr<SWidget> SafeWidget = GetCachedWidget();
			if (SafeWidget.IsValid())
			{
				if (SafeWidget->HasUserFocus(UserIndex.GetValue()))
				{
					FSlateApplication::Get().ClearUserFocus(UserIndex.GetValue());
				}
			}
		}
	}
}
