#include "StevesUI/InputImage.h"
#include "StevesGameSubsystem.h"
#include "StevesUEHelpers.h"
#include "Blueprint/WidgetTree.h"
#include "InputAction.h"

TSharedRef<SWidget> UInputImage::RebuildWidget()
{
    auto Ret = Super::RebuildWidget();

    auto GS = GetStevesGameSubsystem(GetWorld());
    if (GS && !bSubbedToInputEvents)
    {
        bSubbedToInputEvents = true;
        GS->OnInputModeChanged.AddUniqueDynamic(this, &UInputImage::OnInputModeChanged);
        GS->OnButtonInputModeChanged.AddUniqueDynamic(this, &UInputImage::OnInputModeChanged);
        GS->OnAxisInputModeChanged.AddUniqueDynamic(this, &UInputImage::OnInputModeChanged);
        GS->OnEnhancedInputMappingsChanged.AddUniqueDynamic(this, &UInputImage::OnEnhancedInputMappingsChanged);
    }
    
    UpdateImage();

    return Ret;
}

void UInputImage::OnInputModeChanged(int ChangedPlayerIdx, EInputMode InputMode)
{
    if (ChangedPlayerIdx == PlayerIndex)
    {
        // Delay update, in case multiple received in short succession
        MarkImageDirty();
        // auto GS = GetStevesGameSubsystem(GetWorld());
        // UE_LOG(LogTemp, Warning, TEXT("Updating image for input mode change: %s Button device: %s"),
        //     *UEnum::GetValueAsString(InputMode),
        //     *UEnum::GetValueAsString(GS->GetLastInputButtonPressed(ChangedPlayerIdx)));
    }
}

void UInputImage::OnEnhancedInputMappingsChanged()
{
    MarkImageDirty();
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
        GS->OnAxisInputModeChanged.RemoveAll(this);
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

void UInputImage::SetFromInputAction(UInputAction* Action)
{
    BindingType = EInputBindingType::EnhancedInputAction;
    InputAction = Action;
    UpdateImage();
}

void UInputImage::UpdateImage()
{
    auto GS = GetStevesGameSubsystem(GetWorld());
    if (GS)
    {
        UPaperSprite* Sprite = nullptr;
        if (BindingType == EInputBindingType::EnhancedInputAction && !InputAction.IsNull())
        {
            if (auto IA = InputAction.LoadSynchronous())
            {
                Sprite = GS->GetInputImageSpriteFromEnhancedInputAction(IA, DevicePreference, PlayerIndex, GetOwningPlayer(), CustomTheme);
            }
        }
        else
        {
            Sprite = GS->GetInputImageSprite(BindingType, ActionOrAxisName, Key, DevicePreference, PlayerIndex, CustomTheme);    
        }
        
        if (Sprite)
        {
            if (bHiddenBecauseBlank || bOverrideHiddenState)
            {
                // Use Internal so as not to recurse back here
                SetVisibilityInternal(OldVisibility);
                bHiddenBecauseBlank = false;
            }
            // Match size is needed incase size has changed
            // Need to make it update region in case inside a scale box or something else that needs to adjust
            SetBrushFromAtlasInterface(Sprite, true);
        }
        else
        {
            if (IsVisible())
            {
                bHiddenBecauseBlank = true;
                OldVisibility = GetVisibility();
                // Use Internal so as not to recurse back here
                SetVisibilityInternal(ESlateVisibility::Hidden);
            }
        }
    }
    bIsDirty = false;
    DelayUpdate = 0;
}

void UInputImage::MarkImageDirty()
{
    if (UpdateDelay > 0)
    {
        bIsDirty = true;
        DelayUpdate = UpdateDelay;
    }
    else
    {
        UpdateImage();
    }
}

// Tickables
// We need a tick rather than FTimer because timers won't run when game is paused, and UIs are useful while paused! 
ETickableTickType UInputImage::GetTickableTickType() const
{
    return ETickableTickType::Conditional;
}
bool UInputImage::IsTickableWhenPaused() const
{
    // UIs need to update while game is paused
    return true;
}
bool UInputImage::IsTickableInEditor() const
{
    return false;
}

bool UInputImage::IsTickable() const
{
    // Only need to tick while dirty
    return bIsDirty;
}

void UInputImage::Tick(float DeltaTime)
{
    DelayUpdate -= DeltaTime;
    if (DelayUpdate <= 0)
    {
        UpdateImage();
    }
}

TStatId UInputImage::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT( UInputImage, STATGROUP_Tickables );
}

// Tickables
