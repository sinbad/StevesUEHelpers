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
    if (GS)
    {
        GS->OnInputModeChanged.AddUniqueDynamic(this, &UInputImage::OnInputModeChanged);
        CurrentInputMode = GS->GetLastInputModeUsed(PlayerIndex);
        UpdateImage();
    }

    return Ret;
}

void UInputImage::OnInputModeChanged(int ChangedPlayerIdx, EInputMode InputMode)
{
    if (ChangedPlayerIdx == PlayerIndex)
    {
        CurrentInputMode = InputMode;
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
    }
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
        auto Sprite = GS->GetInputImageSprite(BindingType, ActionOrAxisName, Key, PlayerIndex, CustomTheme);
        if (Sprite)
        {
            // Match size is needed incase size has changed
            // Need to make it update region in case inside a scale box or something else that needs to adjust
            SetBrushFromAtlasInterface(Sprite, true);
        }
    }
}
