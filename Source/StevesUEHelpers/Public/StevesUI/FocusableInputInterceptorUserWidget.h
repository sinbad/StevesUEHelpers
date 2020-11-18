#pragma once

#include "CoreMinimal.h"

#include "FocusableUserWidget.h"
#include "Framework/Application/IInputProcessor.h"

#include "FocusableInputInterceptorUserWidget.generated.h"

UCLASS(Blueprintable, BlueprintType)
class STEVESUEHELPERS_API UFocusableInputInterceptorUserWidget : public UFocusableUserWidget
{
    GENERATED_BODY()

    // Nested class which we'll use to poke into input events before anything else eats them
    // Without this it seems impossible to pick up e.g. Gamepad "B" button with UMG up
    // It means we hardcode the controls for menus but that's OK in practice
    class FUiInputPreprocessor : public IInputProcessor, public TSharedFromThis<FUiInputPreprocessor>
    {
    public:
        DECLARE_DELEGATE_RetVal_OneParam(bool, FOnUiKeyDown, const FKeyEvent&);
        FOnUiKeyDown OnUiKeyDown;
        
        virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
        {
            return OnUiKeyDown.Execute(InKeyEvent);
        }
        // Required by IInputProcessor but we don't need
        virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Crs) override{}
    };

protected:
    TSharedPtr<FUiInputPreprocessor> InputPreprocessor;
    
public:

    virtual void NativeConstruct() override;
    virtual void NativeDestruct();

    UFUNCTION()
    virtual bool HandleKeyDownEvent(const FKeyEvent& InKeyEvent);

};
