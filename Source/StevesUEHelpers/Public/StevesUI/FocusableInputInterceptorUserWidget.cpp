#include "FocusableInputInterceptorUserWidget.h"


void UFocusableInputInterceptorUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!InputPreprocessor.IsValid())
    {
        InputPreprocessor = MakeShareable(new FUiInputPreprocessor());
        InputPreprocessor->OnUiKeyDown.BindUObject(this, &UFocusableInputInterceptorUserWidget::HandleKeyDownEvent);
    }
    FSlateApplication::Get().RegisterInputPreProcessor(InputPreprocessor);

}

void UFocusableInputInterceptorUserWidget::NativeDestruct()
{
    Super::NativeDestruct();
    
    FSlateApplication::Get().UnregisterInputPreProcessor(InputPreprocessor);
}


bool UFocusableInputInterceptorUserWidget::HandleKeyDownEvent(const FKeyEvent& InKeyEvent)
{
    // Do nothing by default
    return false;
}
