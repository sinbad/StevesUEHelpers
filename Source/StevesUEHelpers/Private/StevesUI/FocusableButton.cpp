#include "StevesUI/FocusableButton.h"
#include "StevesUI/SFocusableButton.h"
#include "Components/ButtonSlot.h"
#include "Framework/Application/NavigationConfig.h"
#include "Framework/Application/SlateApplication.h"


UFocusableButton::UFocusableButton(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UFocusableButton::RebuildWidget()
{
    MyButton = SNew(SFocusableButton)
            // This part is standard button behaviour
            .OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleClicked))
            .OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandlePressed))
            .OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleReleased))
            .OnHovered_UObject( this, &ThisClass::SlateHandleHovered )
            .OnUnhovered_UObject( this, &ThisClass::SlateHandleUnhovered )
            .ButtonStyle(&GetStyle())
            .ClickMethod(GetClickMethod())
            .TouchMethod(GetTouchMethod())
            .PressMethod(GetPressMethod())
            .IsFocusable(GetIsFocusable())
            // Our new events
            .OnFocusReceived(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleFocusReceived))
            .OnFocusLost(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleFocusLost))
    ;

    if ( GetChildrenCount() > 0 )
    {
        Cast<UButtonSlot>(GetContentSlot())->BuildSlot(MyButton.ToSharedRef());
    }

    RefreshFocussedStyle();

    return MyButton.ToSharedRef();    
}

void UFocusableButton::RefreshFocussedStyle_Implementation()
{
    // Copy Widget style but make normal same as hovered    
    FocussedStyle = GetStyle();
    FocussedStyle.Normal = FocussedStyle.Hovered;
}

void UFocusableButton::SimulatePress()
{
    if (MyButton)
    {
        // In order to get the visual change we need to call SButton::Press() and SButton::Release(), but they're both private
        // The only public method we can use to simulate the click properly is OnKeyDown/Up or OnMouseButtonDown/Up
        // Use Virtual_Accept since that is general
        FKeyEvent KeyEvent(
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
	        EKeys::Virtual_Gamepad_Accept.GetVirtualKey(),
#else
	        EKeys::Virtual_Accept,
#endif
	        FModifierKeysState(), 0, false, 0, 0);
        MyButton->OnKeyDown(MyButton->GetCachedGeometry(), KeyEvent);
    }
}

void UFocusableButton::SimulateRelease()
{
    if (MyButton)
    {
        // In order to get the visual change we need to call SButton::Press() and SButton::Release(), but they're both private
        // The only public method we can use to simulate the click properly is OnKeyDown/Up or OnMouseButtonDown/Up
        // Use Virtual_Accept since that is general
        FKeyEvent KeyEvent(
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
			EKeys::Virtual_Gamepad_Accept.GetVirtualKey(),
#else
			EKeys::Virtual_Accept,
#endif
			FModifierKeysState(), 0, false, 0, 0);
        MyButton->OnKeyUp(MyButton->GetCachedGeometry(), KeyEvent);

    }
}

void UFocusableButton::SlateHandleFocusReceived()
{
    ApplyFocusStyle();
    OnFocusReceived.Broadcast();
}

void UFocusableButton::SlateHandleFocusLost()
{
    UndoFocusStyle();
    OnFocusLost.Broadcast();
}


void UFocusableButton::ApplyFocusStyle()
{
    if (MyButton.IsValid() && bUseHoverStyleWhenFocussed)
    {
        MyButton->SetButtonStyle(&FocussedStyle);
    }
}

void UFocusableButton::UndoFocusStyle()
{
    if (MyButton.IsValid())
    {
        MyButton->SetButtonStyle(&GetStyle());
    }
}

void UFocusableButton::SlateHandleHovered()
{
    if (bTakeFocusOnHover)
    {
        SetFocus();
    }
    OnHovered.Broadcast();
}

void UFocusableButton::SlateHandleUnhovered()
{
	if (bLoseFocusOnUnhover)
	{
		Unfocus();
	}
	OnUnhovered.Broadcast();
}

void UFocusableButton::Unfocus() const
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
