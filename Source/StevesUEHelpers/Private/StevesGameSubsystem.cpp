#include "StevesGameSubsystem.h"
#include "StevesGameViewportClientBase.h"
#include "StevesUEHelpers.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "StevesUI/KeySprite.h"

//PRAGMA_DISABLE_OPTIMIZATION

void UStevesGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    CreateInputDetector();
    InitTheme();
    InitForegroundCheck();
}

void UStevesGameSubsystem::Deinitialize()
{
    Super::Deinitialize();
    DestroyInputDetector();
}


void UStevesGameSubsystem::CreateInputDetector()
{
    if (!InputDetector.IsValid())
    {
        InputDetector = MakeShareable(new FInputModeDetector());
        FSlateApplication::Get().RegisterInputPreProcessor(InputDetector);

        InputDetector->OnInputModeChanged.BindUObject(this, &UStevesGameSubsystem::OnInputDetectorModeChanged);
    }

}

void UStevesGameSubsystem::DestroyInputDetector()
{
    if (InputDetector.IsValid())
    {
        FSlateApplication::Get().UnregisterInputPreProcessor(InputDetector);
        InputDetector.Reset();
    }
}

void UStevesGameSubsystem::InitTheme()
{
    DefaultUiTheme = LoadObject<UUiTheme>(nullptr, *DefaultUiThemePath, nullptr);
}


void UStevesGameSubsystem::InitForegroundCheck()
{
    // Check foreground status every 0.5 seconds
    GetWorld()->GetTimerManager().SetTimer(ForegroundCheckHandle, this, &UStevesGameSubsystem::CheckForeground, 0.5);
}

void UStevesGameSubsystem::CheckForeground()
{
    bool bNewForeground = bIsForeground;

    if (IsValid(GEngine) && IsValid(GEngine->GameViewport) && GEngine->GameViewport->Viewport)
        bNewForeground = GEngine->GameViewport->Viewport->IsForegroundWindow();

    if (bNewForeground != bIsForeground)
    {
        bIsForeground = bNewForeground;
        InputDetector->bIgnoreEvents = !bIsForeground;
        
        OnWindowForegroundChanged.Broadcast(bIsForeground);
    }

    
}
void UStevesGameSubsystem::OnInputDetectorModeChanged(int PlayerIndex, EInputMode NewMode)
{
    // We can't check this during Initialize because it's too early
    if (!bCheckedViewportClient)
    {
        auto GI = GetGameInstance();        
        auto VC = Cast<UStevesGameViewportClientBase>(GI->GetGameViewportClient());
        if (!VC)
            UE_LOG(LogStevesUEHelpers, Warning, TEXT("Your GameViewportClient needs to be set to a subclass of UStevesGameViewportClientBase if you want full functionality!"))

        bCheckedViewportClient = true;
    
    }
    
    auto GI = GetGameInstance();
    auto VC = GI->GetGameViewportClient();
    auto SVC = Cast<UStevesGameViewportClientBase>(VC);
    if (VC)
    {
        if (NewMode == EInputMode::Gamepad)
        {
            // First move mouse pointer out of the way because it still generates mouse hits (unless we make source changes to Slate, ugh)
            auto PC = GI->GetFirstLocalPlayerController();
            FVector2D Sz;
            VC->GetViewportSize(Sz);
            // -1 because if you move cursor outside window when captured, Slate blows up when you press Return, ughghh 
            PC->SetMouseLocation(Sz.X-1,Sz.Y-1);

            // Now hide it
            // I've seen people use PC->bShowMouseCursor but this messes with capturing when you switch back & forth
            // especially when pausing in the editor

            // instead, I'm using a separate flag to suppress it, see UiFixGameViewportClient for usage
            if (SVC)
                SVC->SetSuppressMouseCursor(true);
        }
        else if (NewMode == EInputMode::Mouse)
        {
            if (SVC)
                SVC->SetSuppressMouseCursor(false);
        }
    }
    OnInputModeChanged.Broadcast(PlayerIndex, NewMode);
}

FFocusSystem* UStevesGameSubsystem::GetFocusSystem()
{
    return &FocusSystem;
}

UPaperSprite* UStevesGameSubsystem::GetInputImageSprite(EInputBindingType BindingType, FName ActionOrAxis,
    FKey Key, int PlayerIdx, const UUiTheme* Theme)
{
    switch(BindingType)
    {
    case EInputBindingType::Action:
        return GetInputImageSpriteFromAction(ActionOrAxis, PlayerIdx, Theme);
    case EInputBindingType::Axis:
        return GetInputImageSpriteFromAxis(ActionOrAxis, PlayerIdx, Theme);
    case EInputBindingType::Key:
        return GetInputImageSpriteFromKey(Key, PlayerIdx, Theme);
    default:
        return nullptr;
    }
}

// This is not threadsafe! But only used in UI thread in practice
TArray<FInputActionKeyMapping> GS_TempActionMap;
TArray<FInputAxisKeyMapping> GS_TempAxisMap;

UPaperSprite* UStevesGameSubsystem::GetInputImageSpriteFromAction(const FName& Name, int PlayerIdx, const UUiTheme* Theme)
{
    
    // Look up the key for this action
    UInputSettings* Settings = UInputSettings::GetInputSettings();
    GS_TempActionMap.Empty();
    Settings->GetActionMappingByName(Name, GS_TempActionMap);
    const bool WantGamepad = LastInputWasGamePad(PlayerIdx);
    for (auto && ActionMap : GS_TempActionMap)
    {
        if (ActionMap.Key.IsGamepadKey() == WantGamepad)
        {
            return GetInputImageSpriteFromKey(ActionMap.Key, PlayerIdx, Theme);
        }
    }
    // if we fell through, didn't find a mapping which matched our gamepad preference
    if (GS_TempActionMap.Num())
    {
        return GetInputImageSpriteFromKey(GS_TempActionMap[0].Key, PlayerIdx, Theme);
    }
    return nullptr;
}

UPaperSprite* UStevesGameSubsystem::GetInputImageSpriteFromAxis(const FName& Name, int PlayerIdx, const UUiTheme* Theme)
{
    // Look up the key for this axis
    UInputSettings* Settings = UInputSettings::GetInputSettings();
    GS_TempAxisMap.Empty();
    Settings->GetAxisMappingByName(Name, GS_TempAxisMap);
    const bool WantGamepad = LastInputWasGamePad(PlayerIdx);
    for (auto && AxisMap : GS_TempAxisMap)
    {
        if (AxisMap.Key.IsGamepadKey() == WantGamepad)
        {
            return GetInputImageSpriteFromKey(AxisMap.Key, PlayerIdx, Theme);
        }
    }
    // if we fell through, didn't find a mapping which matched our gamepad preference
    if (GS_TempAxisMap.Num())
    {
        return GetInputImageSpriteFromKey(GS_TempAxisMap[0].Key, PlayerIdx, Theme);
    }    
    return nullptr;
}

TSoftObjectPtr<UDataTable> UStevesGameSubsystem::GetGamepadImages(int PlayerIndex, const UUiTheme* Theme)
{
    // TODO: determine type of controller
    return Theme->XboxControllerImages;    
}

UPaperSprite* UStevesGameSubsystem::GetInputImageSpriteFromKey(const FKey& InKey, int PlayerIndex, const UUiTheme* Theme)
{
    if (!IsValid(Theme))
        Theme = GetDefaultUiTheme();
    
    if (Theme)
    {
        if (InKey.IsGamepadKey())
            return GetImageSpriteFromTable(InKey,  GetGamepadImages(PlayerIndex, Theme));
        else
            return GetImageSpriteFromTable(InKey, Theme->KeyboardMouseImages);
    }

    return nullptr;
}


UPaperSprite* UStevesGameSubsystem::GetImageSpriteFromTable(const FKey& InKey,
    const TSoftObjectPtr<UDataTable>& Asset)
{
    // Sync load for simplicity for now
    const auto Table = Asset.LoadSynchronous();
    // Rows are named the same as the key name
    const auto SpriteRow = Table->FindRow<FKeySprite>(InKey.GetFName(), "Find Key Image");
    if (SpriteRow)
    {
        return SpriteRow->Sprite;
    }
    return nullptr;
}

void UStevesGameSubsystem::SetBrushFromAtlas(FSlateBrush* Brush, TScriptInterface<ISlateTextureAtlasInterface> AtlasRegion, bool bMatchSize)
{
    if(Brush->GetResourceObject() != AtlasRegion.GetObject())
    {
        Brush->SetResourceObject(AtlasRegion.GetObject());

        if (bMatchSize)
        {
            if (AtlasRegion)
            {
                const FSlateAtlasData AtlasData = AtlasRegion->GetSlateAtlasData();
                Brush->ImageSize = AtlasData.GetSourceDimensions();
            }
            else
            {
                Brush->ImageSize = FVector2D(0, 0);
            }
        }
    }
}


bool UStevesGameSubsystem::FInputModeDetector::ShouldProcessInputEvents() const
{
    return !bIgnoreEvents;
}

UStevesGameSubsystem::FInputModeDetector::FInputModeDetector()
{
    // 4 local players should be plenty usually (will expand if necessary)
    LastInputModeByPlayer.Init(EInputMode::Mouse, 4);
}

bool UStevesGameSubsystem::FInputModeDetector::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
    if (ShouldProcessInputEvents())
    {
        // Key down also registers for gamepad buttons
        ProcessKeyOrButton(InKeyEvent.GetUserIndex(), InKeyEvent.GetKey());
    }

    // Don't consume
    return false;
}

bool UStevesGameSubsystem::FInputModeDetector::HandleAnalogInputEvent(FSlateApplication& SlateApp,
    const FAnalogInputEvent& InAnalogInputEvent)
{
    if (ShouldProcessInputEvents())
    {
        if (InAnalogInputEvent.GetAnalogValue() > GamepadAxisThreshold)
            SetMode(InAnalogInputEvent.GetUserIndex(), EInputMode::Gamepad);
    }
    
    // Don't consume
    return false;
}

bool UStevesGameSubsystem::FInputModeDetector::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    if (ShouldProcessInputEvents())
    {
        FVector2D Dist = MouseEvent.GetScreenSpacePosition() - MouseEvent.GetLastScreenSpacePosition();
        if (FMath::Abs(Dist.X) > MouseMoveThreshold || FMath::Abs(Dist.Y) > MouseMoveThreshold)
        {
            SetMode(MouseEvent.GetUserIndex(), EInputMode::Mouse);
        }
    }
    // Don't consume
    return false;
}

bool UStevesGameSubsystem::FInputModeDetector::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    if (ShouldProcessInputEvents())
    {
        // We don't care which button
        SetMode(MouseEvent.GetUserIndex(), EInputMode::Mouse);
    }
    
    // Don't consume
    return false;
}

bool UStevesGameSubsystem::FInputModeDetector::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent,
    const FPointerEvent* InGestureEvent)
{
    if (ShouldProcessInputEvents())
    {
        SetMode(InWheelEvent.GetUserIndex(), EInputMode::Mouse);
    }
    
    // Don't consume
    return false;
}

EInputMode UStevesGameSubsystem::FInputModeDetector::GetLastInputMode(int PlayerIndex)
{
    if (PlayerIndex >= 0 && PlayerIndex < LastInputModeByPlayer.Num())
        return LastInputModeByPlayer[PlayerIndex];

    // Assume default if never told
    return DefaultInputMode;
}


void UStevesGameSubsystem::FInputModeDetector::ProcessKeyOrButton(int PlayerIndex, FKey Key)
{
    if (Key.IsGamepadKey())
    {
        SetMode(PlayerIndex, EInputMode::Gamepad);
    }
    else if (Key.IsMouseButton())
    {
        // Assuming mice don't have analog buttons!
        SetMode(PlayerIndex, EInputMode::Mouse);
    }
    else
    {
        // We assume anything that's not mouse and not gamepad is a keyboard
        // Assuming keyboards don't have analog buttons!
        SetMode(PlayerIndex, EInputMode::Keyboard);
    }

}
	
void UStevesGameSubsystem::FInputModeDetector::SetMode(int PlayerIndex, EInputMode NewMode)
{
    if (NewMode != EInputMode::Unknown && NewMode != GetLastInputMode(PlayerIndex))
    {
        if (PlayerIndex >= LastInputModeByPlayer.Num())
            LastInputModeByPlayer.SetNum(PlayerIndex + 1);
        
        LastInputModeByPlayer[PlayerIndex] = NewMode;
        OnInputModeChanged.ExecuteIfBound(PlayerIndex, NewMode);
        //UE_LOG(LogStevesUEHelpers, Display, TEXT("Input mode for player %d changed: %s"), PlayerIndex, *UEnum::GetValueAsString(NewMode));
    }
}

//PRAGMA_ENABLE_OPTIMIZATION