#include "StevesUI/InputImage.h"
#include "StevesUI.h"
#include "StevesUI/KeySprite.h"
#include "StevesGameSubsystem.h"
#include "StevesUEHelpers.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/AssetManager.h"
#include "GameFramework/InputSettings.h"

TSharedRef<SWidget> UInputImage::RebuildWidget()
{
    auto Ret = Super::RebuildWidget();

    auto GS = GetStevesGameSubsystem(GetWorld());
    if (GS)
    {
        GS->OnInputModeChanged.AddUniqueDynamic(this, &UInputImage::OnInputModeChanged);
        CurrentInputMode = GS->GetLastInputModeUsed();
        UpdateImage();
    }

    return Ret;
}

void UInputImage::OnInputModeChanged(int PlayerIndex, EInputMode InputMode)
{
    CurrentInputMode = InputMode;
    UpdateImage();
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
    switch(BindingType)
    {
    case EInputBindingType::Action:
        UpdateImageFromAction(ActionOrAxisName);
        break;
    case EInputBindingType::Axis:
        UpdateImageFromAxis(ActionOrAxisName);
        break;
    case EInputBindingType::Key:
        UpdateImageFromKey(Key);
        break;
    default: ;
    }
    
}
TArray<FInputActionKeyMapping> TempActionMap;
TArray<FInputAxisKeyMapping> TempAxisMap;

void UInputImage::UpdateImageFromAction(const FName& Name)
{
    // Look up the key for this action
    UInputSettings* Settings = UInputSettings::GetInputSettings();
    TempActionMap.Empty();
    Settings->GetActionMappingByName(Name, TempActionMap);
    const bool WantGamepad = CurrentInputMode == EInputMode::Gamepad;
    for (auto && ActionMap : TempActionMap)
    {
        if (ActionMap.Key.IsGamepadKey() == WantGamepad)
        {
            UpdateImageFromKey(ActionMap.Key);
            return;
        }
    }
    // if we fell through, didn't find a mapping which matched our gamepad preference
    if (TempActionMap.Num())
    {
        UpdateImageFromKey(TempActionMap[0].Key);
    }    
}

void UInputImage::UpdateImageFromAxis(const FName& Name)
{
    // Look up the key for this axis
    UInputSettings* Settings = UInputSettings::GetInputSettings();
    TempAxisMap.Empty();
    Settings->GetAxisMappingByName(Name, TempAxisMap);
    const bool WantGamepad = CurrentInputMode == EInputMode::Gamepad;
    for (auto && AxisMap : TempAxisMap)
    {
        if (AxisMap.Key.IsGamepadKey() == WantGamepad)
        {
            UpdateImageFromKey(AxisMap.Key);
            return;
        }
    }
    // if we fell through, didn't find a mapping which matched our gamepad preference
    if (TempAxisMap.Num())
    {
        UpdateImageFromKey(TempAxisMap[0].Key);
    }    
}

void UInputImage::UpdateImageFromKey(const FKey& InKey)
{
    auto T = GetTheme();
    if (T)
    {
        if (InKey.IsGamepadKey())
            UpdateImageFromTable(InKey, T->XboxControllerImages);
        else
            UpdateImageFromTable(InKey, T->KeyboardMouseImages);
    }        
}

void UInputImage::UpdateImageFromTable(const FKey& InKey, const TSoftObjectPtr<UDataTable>& Asset)
{
    // Sync load for simplicity for now
    const auto Table = Asset.LoadSynchronous();
    // Rows are named the same as the key name
    const auto SpriteRow = Table->FindRow<FKeySprite>(InKey.GetFName(), "Find Key Image");
    if (SpriteRow)
        SetBrushFromAtlasInterface(SpriteRow->Sprite);
}

UUiTheme* UInputImage::GetTheme()
{
    if (IsValid(CustomTheme))
        return CustomTheme;

    auto GS = GetStevesGameSubsystem(GetWorld());
    if (GS)
        return GS->GetDefaultUiTheme();

    return nullptr;
}
