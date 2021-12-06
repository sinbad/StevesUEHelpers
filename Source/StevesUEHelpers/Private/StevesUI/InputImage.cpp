#include "StevesUI/InputImage.h"
#include "StevesUI/KeySprite.h"
#include "StevesGameSubsystem.h"
#include "StevesUEHelpers.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/AssetManager.h"

TSharedRef<SWidget> UInputImage::RebuildWidget()
{
    auto Ret = Super::RebuildWidget();

    auto GS = GetStevesGameSubsystem(GetWorld());
    if (GS && !bSubbedToInputEvents)
    {
        bSubbedToInputEvents = true;
        GS->OnInputModeChanged.AddUniqueDynamic(this, &UInputImage::OnInputModeChanged);
        GS->OnButtonInputModeChanged.AddUniqueDynamic(this, &UInputImage::OnInputModeChanged);
    }
    UpdateImage();

    return Ret;
}

void UInputImage::OnInputModeChanged(int ChangedPlayerIdx, EInputMode InputMode)
{
    if (ChangedPlayerIdx == PlayerIndex)
    {
        // auto GS = GetStevesGameSubsystem(GetWorld());
        // UE_LOG(LogTemp, Warning, TEXT("Updating image for input mode change: %s Button device: %s"),
        //     *UEnum::GetValueAsString(InputMode),
        //     *UEnum::GetValueAsString(GS->GetLastInputButtonPressed(ChangedPlayerIdx)));
        UpdateImage();
    }
}

void UInputImage::SetCustomTheme(UUiTheme* Theme)
{
    CustomTheme = Theme;
    UpdateImage();
}

void UInputImage::BeginDestroy()
{
    Super::BeginDestroy();
    
    auto GS = GetStevesGameSubsystem(GetWorld());
    if (GS)
    {
        GS->OnInputModeChanged.RemoveAll(this);
        GS->OnButtonInputModeChanged.RemoveAll(this);
    }
}

void UInputImage::SetVisibility(ESlateVisibility InVisibility)
{
    Super::SetVisibility(InVisibility);

    switch(InVisibility)
    {
    case ESlateVisibility::Collapsed:
    case ESlateVisibility::Hidden:
        break;
    default:
    case ESlateVisibility::Visible: 
    case ESlateVisibility::HitTestInvisible:
    case ESlateVisibility::SelfHitTestInvisible:
        // Make sure we update when our visibility is changed
        UpdateImage();
        break;
    };
}

void UInputImage::SetFromAction(FName Name)
{
    BindingType = EInputBindingType::Action;
    ActionOrAxisName = Name;
    UpdateImage();
}

void UInputImage::SetFromAxis(FName Name)
{
    BindingType = EInputBindingType::Axis;
    ActionOrAxisName = Name;
    UpdateImage();
}

void UInputImage::SetFromKey(FKey K)
{
    BindingType = EInputBindingType::Key;
    Key = K;
    UpdateImage();
}

void UInputImage::UpdateImage()
{
    auto GS = GetStevesGameSubsystem(GetWorld());
    if (GS)
    {
        auto Sprite = GS->GetInputImageSprite(BindingType, ActionOrAxisName, Key, DevicePreference, PlayerIndex, CustomTheme);
        if (Sprite)
        {
            // Match size is needed incase size has changed
            // Need to make it update region in case inside a scale box or something else that needs to adjust
            SetBrushFromAtlasInterface(Sprite, true);
        }
    }
}
