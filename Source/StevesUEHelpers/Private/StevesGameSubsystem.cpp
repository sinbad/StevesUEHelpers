#include "StevesGameSubsystem.h"

#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "StevesGameViewportClientBase.h"
#include "StevesPluginSettings.h"
#include "StevesUEHelpers.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "StevesUI/KeySprite.h"
#include "StevesUI/StevesUI.h"

//PRAGMA_DISABLE_OPTIMIZATION

void UStevesGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

#if !UE_SERVER
    CreateInputDetector();
    InitTheme();
    InitForegroundCheck();
    NotifyEnhancedInputMappingsChanged();
#endif
}

void UStevesGameSubsystem::Deinitialize()
{
    Super::Deinitialize();
#if !UE_SERVER
    DestroyInputDetector();
#endif
}


void UStevesGameSubsystem::CreateInputDetector()
{
#if !UE_SERVER
    if (!InputDetector.IsValid())
    {
        InputDetector = MakeShareable(new FInputModeDetector());
        FSlateApplication::Get().RegisterInputPreProcessor(InputDetector);

        InputDetector->OnInputModeChanged.BindUObject(this, &UStevesGameSubsystem::OnInputDetectorModeChanged);
        InputDetector->OnButtonInputModeChanged.BindUObject(this, &UStevesGameSubsystem::OnButtonInputDetectorModeChanged);
        InputDetector->OnAxisInputModeChanged.BindUObject(this, &UStevesGameSubsystem::OnAxisInputDetectorModeChanged);
    }
#endif
}

void UStevesGameSubsystem::DestroyInputDetector()
{
#if !UE_SERVER
    if (InputDetector.IsValid())
    {
        FSlateApplication::Get().UnregisterInputPreProcessor(InputDetector);
        InputDetector.Reset();
    }
#endif
}

void UStevesGameSubsystem::NotifyEnhancedInputMappingsChanged()
{
    // delay to ensure there's a tick in between which updates the mappings, it's not synchronous
    auto DelayedFunc = [this]()
    {
        OnEnhancedInputMappingsChanged.Broadcast();
    };
    FTimerHandle TempHandle;
    GetWorld()->GetTimerManager().SetTimer(TempHandle, FTimerDelegate::CreateLambda(DelayedFunc), 0.05, false);
}

TSoftObjectPtr<UInputAction> UStevesGameSubsystem::FindEnhancedInputAction(const FString& Name)
{
    if (FAssetRegistryModule* AssetRegistryModule = FModuleManager::LoadModulePtr<FAssetRegistryModule>(TEXT("AssetRegistry")))
    {
        IAssetRegistry& AssetRegistry = AssetRegistryModule->Get();
        if (auto Settings = GetDefault<UStevesPluginSettings>())
        {
            for (const auto& Dir : Settings->EnhancedInputActionSearchDirectories)
            {
                if (!FPackageName::IsValidPath(Dir.Path))
                {
                    continue;
                }

                TArray<FAssetData> Assets;
                FString Package = FPaths::Combine(Dir.Path, Name);
                if (AssetRegistry.GetAssetsByPackageName(FName(*Package), Assets, true))
                {
                    for (const FAssetData& Asset : Assets)
                    {
                        if (Asset.GetClass() == UInputAction::StaticClass())
                        {
                            return TSoftObjectPtr<UInputAction>(Asset.GetSoftObjectPath());
                        }
                    }
                }

            }
        }
    }
    return nullptr;
}

void UStevesGameSubsystem::RegisterInterestInEnhancedInputAction(const UInputAction* Action, ETriggerEvent TriggerEvent)
{
    // Avoid registering duplicate interest
    FEnhancedInputInterest Interest(Action, TriggerEvent);
    if (!RegisteredEnhancedInputActionInterests.Contains(Interest))
    {
        if (auto GI = GetGameInstance())
        {
            if (auto PC = GI->GetFirstLocalPlayerController())
            {
                if (auto EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
                {
                    EIC->BindAction(Action, TriggerEvent, this, &ThisClass::EnhancedInputActionTriggered);
                    RegisteredEnhancedInputActionInterests.Add(Interest);
                }
            }
        }
    }
    
}

void UStevesGameSubsystem::EnhancedInputActionTriggered(const FInputActionInstance& InputActionInstance)
{
    OnEnhancedInputActionTriggered.Broadcast(InputActionInstance.GetSourceAction(), InputActionInstance.GetTriggerEvent());
}



void UStevesGameSubsystem::InitTheme()
{
    DefaultUiTheme = LoadObject<UUiTheme>(nullptr, *DefaultUiThemePath, nullptr);
}


void UStevesGameSubsystem::InitForegroundCheck()
{
    // Check foreground status every 0.5 seconds
    GetWorld()->GetTimerManager().SetTimer(ForegroundCheckHandle, this, &UStevesGameSubsystem::CheckForeground, 0.5, true);
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
            UE_LOG(LogStevesUEHelpers, Log, TEXT("Consider using UStevesGameViewportClientBase for your GameViewportClient"))

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

void UStevesGameSubsystem::OnButtonInputDetectorModeChanged(int PlayerIndex, EInputMode NewMode)
{
    // This is specifically for button changes; if this is a different main input mode it will also be registered in OnInputDetectorModeChanged
    // Just relay this one
    OnButtonInputModeChanged.Broadcast(PlayerIndex, NewMode);
}

void UStevesGameSubsystem::OnAxisInputDetectorModeChanged(int PlayerIndex, EInputMode NewMode)
{
    // This is specifically for button changes; if this is a different main input mode it will also be registered in OnInputDetectorModeChanged
    // Just relay this one
    OnAxisInputModeChanged.Broadcast(PlayerIndex, NewMode);
}

FFocusSystem* UStevesGameSubsystem::GetFocusSystem()
{
    return &FocusSystem;
}

UPaperSprite* UStevesGameSubsystem::GetInputImageSprite(EInputBindingType BindingType,
                                                        FName ActionOrAxis,
                                                        FKey Key,
                                                        EInputImageDevicePreference DevicePreference,
                                                        int PlayerIdx,
                                                        const UUiTheme* Theme)
{
    switch(BindingType)
    {
    case EInputBindingType::Action:
        return GetInputImageSpriteFromAction(ActionOrAxis, DevicePreference, PlayerIdx, Theme);
    case EInputBindingType::Axis:
        return GetInputImageSpriteFromAxis(ActionOrAxis, DevicePreference, PlayerIdx, Theme);
    case EInputBindingType::Key:
        return GetInputImageSpriteFromKey(Key, PlayerIdx, Theme);
    default:
        return nullptr;
    }
}

// This is not threadsafe! But only used in UI thread in practice
TArray<FInputActionKeyMapping> GS_TempActionMap;
TArray<FInputAxisKeyMapping> GS_TempAxisMap;

UPaperSprite* UStevesGameSubsystem::GetInputImageSpriteFromAction(const FName& Name,
                                                                  EInputImageDevicePreference DevicePreference,
                                                                  int PlayerIdx,
                                                                  const UUiTheme* Theme)
{
    UInputSettings* Settings = UInputSettings::GetInputSettings();
    GS_TempActionMap.Empty();
    Settings->GetActionMappingByName(Name, GS_TempActionMap);
    
    // For default, prefer latest press keyboard/mouse for buttons
    if (DevicePreference == EInputImageDevicePreference::Auto)
        DevicePreference = EInputImageDevicePreference::Gamepad_Keyboard_Mouse_Button;

    const EInputMode LastInput = GetLastInputModeUsed(PlayerIdx);
    const EInputMode LastButtonInput = GetLastInputButtonPressed(PlayerIdx);
    const EInputMode LastAxisInput = GetLastInputAxisMoved(PlayerIdx);
    const auto Preferred = GetPreferedActionOrAxisMapping<FInputActionKeyMapping>(GS_TempActionMap, Name, DevicePreference, LastInput, LastButtonInput, LastAxisInput);
    if (Preferred)
    {
        return GetInputImageSpriteFromKey(Preferred->Key, PlayerIdx, Theme);
    }
    return nullptr;
}

UPaperSprite* UStevesGameSubsystem::GetInputImageSpriteFromAxis(const FName& Name,
                                                                EInputImageDevicePreference DevicePreference,
                                                                int PlayerIdx,
                                                                const UUiTheme* Theme)
{
    // Look up the key for this axis
    UInputSettings* Settings = UInputSettings::GetInputSettings();
    GS_TempAxisMap.Empty();
    Settings->GetAxisMappingByName(Name, GS_TempAxisMap);

    // For default, prefer mouse for axes
    if (DevicePreference == EInputImageDevicePreference::Auto)
        DevicePreference = EInputImageDevicePreference::Gamepad_Mouse_Keyboard;

    const EInputMode LastInput = GetLastInputModeUsed(PlayerIdx);
    const EInputMode LastButtonInput = GetLastInputButtonPressed(PlayerIdx);
    const EInputMode LastAxisInput = GetLastInputAxisMoved(PlayerIdx);
    const auto Preferred = GetPreferedActionOrAxisMapping<FInputAxisKeyMapping>(GS_TempAxisMap, Name, DevicePreference, LastInput, LastButtonInput, LastAxisInput);
    if (Preferred)
    {
        return GetInputImageSpriteFromKey(Preferred->Key, PlayerIdx, Theme);
    }
    return nullptr;
}

UPaperSprite* UStevesGameSubsystem::GetInputImageSpriteFromEnhancedInputAction(UInputAction* Action,
    EInputImageDevicePreference DevicePreference,
    int PlayerIdx,
    APlayerController* PC,
    UUiTheme* Theme)
{
    
    if (const UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    {
        const TArray<FKey> Keys = Subsystem->QueryKeysMappedToAction(Action);

        // For default, prefer mouse for axes
        if (DevicePreference == EInputImageDevicePreference::Auto)
        {
            if (Action->ValueType == EInputActionValueType::Boolean)
            {
                DevicePreference = EInputImageDevicePreference::Gamepad_Keyboard_Mouse_Button;
            }
            else
            {
                DevicePreference = EInputImageDevicePreference::Gamepad_Mouse_Keyboard;
            }
        }
        const EInputMode LastInput = GetLastInputModeUsed(PlayerIdx);
        const EInputMode LastButtonInput = GetLastInputButtonPressed(PlayerIdx);
        const EInputMode LastAxisInput = GetLastInputAxisMoved(PlayerIdx);
        if (const FKey* PreferredKey = GetPreferedKeyMapping(Keys, DevicePreference, LastInput, LastButtonInput, LastAxisInput))
        {
            return GetInputImageSpriteFromKey(*PreferredKey, PlayerIdx, Theme);
        }
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

FStevesTextureRenderTargetPoolPtr UStevesGameSubsystem::GetTextureRenderTargetPool(FName Name, bool bAutoCreate)
{
    // On the assumption there won't be *loads* of pools, not worth a map, just iterate
    for (auto Tex : TextureRenderTargetPools)
    {
        if (Tex->GetName() == Name)
            return Tex;
    }

    if (bAutoCreate)
    {
        FStevesTextureRenderTargetPoolPtr Pool = MakeShared<FStevesTextureRenderTargetPool>(Name, this);
        TextureRenderTargetPools.Add(Pool);
        return Pool;
    }

    return nullptr;
    
}


bool UStevesGameSubsystem::FInputModeDetector::ShouldProcessInputEvents() const
{
    return !bIgnoreEvents;
}

UStevesGameSubsystem::FInputModeDetector::FInputModeDetector()
{
    // 4 local players should be plenty usually (will expand if necessary)
    LastInputModeByPlayer.Init(DefaultInputMode, 4);
    LastButtonPressByPlayer.Init(DefaultButtonInputMode, 4);
    LastAxisMoveByPlayer.Init(DefaultAxisInputMode, 4);
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
            SetMode(InAnalogInputEvent.GetUserIndex(), EInputMode::Gamepad, false);
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
            SetMode(MouseEvent.GetUserIndex(), EInputMode::Mouse, false);
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
        SetMode(MouseEvent.GetUserIndex(), EInputMode::Mouse, true);
    }
    
    // Don't consume
    return false;
}

bool UStevesGameSubsystem::FInputModeDetector::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent,
    const FPointerEvent* InGestureEvent)
{
    if (ShouldProcessInputEvents())
    {
        SetMode(InWheelEvent.GetUserIndex(), EInputMode::Mouse, false);
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

EInputMode UStevesGameSubsystem::FInputModeDetector::GetLastButtonInputMode(int PlayerIndex)
{
    if (PlayerIndex >= 0 && PlayerIndex < LastButtonPressByPlayer.Num())
        return LastButtonPressByPlayer[PlayerIndex];

    // Assume default if never told
    return DefaultButtonInputMode;
}

EInputMode UStevesGameSubsystem::FInputModeDetector::GetLastAxisInputMode(int PlayerIndex)
{
    if (PlayerIndex >= 0 && PlayerIndex < LastAxisMoveByPlayer.Num())
        return LastAxisMoveByPlayer[PlayerIndex];

    // Assume default if never told
    return DefaultAxisInputMode;
}

void UStevesGameSubsystem::FInputModeDetector::ProcessKeyOrButton(int PlayerIndex, FKey Key)
{
    if (Key.IsGamepadKey())
    {
        SetMode(PlayerIndex, EInputMode::Gamepad, IsAGamepadButton(Key));
    }
    else if (Key.IsMouseButton())
    {
        // Assuming mice don't have analog buttons!
        SetMode(PlayerIndex, EInputMode::Mouse, true);
    }
    else
    {
        // We assume anything that's not mouse and not gamepad is a keyboard
        // Assuming keyboards don't have analog buttons!
        SetMode(PlayerIndex, EInputMode::Keyboard, true);
    }

}

bool UStevesGameSubsystem::FInputModeDetector::IsAGamepadButton(const FKey& Key)
{
    
    // Key.IsButtonAxis() returns true for some thumbstick movement events, because the axis type is EInputAxisType::Button for
    // some reason. That means you get button events for thumbstick movements, which is super dumb. 
    // See core engine InputCoreTypes.cpp for the stick axes which are defined FKeyDetails::GamepadKey | FKeyDetails::ButtonAxis
    // This is for some kind of virtual input but it's a nasty hack, omit them
    return Key.IsGamepadKey() &&
        Key != EKeys::Gamepad_LeftStick_Up &&
        Key != EKeys::Gamepad_LeftStick_Down &&
        Key != EKeys::Gamepad_LeftStick_Left &&
        Key != EKeys::Gamepad_LeftStick_Right &&
        Key != EKeys::Gamepad_RightStick_Up &&
        Key != EKeys::Gamepad_RightStick_Down &&
        Key != EKeys::Gamepad_RightStick_Left &&
        Key != EKeys::Gamepad_RightStick_Right;

}

void UStevesGameSubsystem::FInputModeDetector::SetMode(int PlayerIndex, EInputMode NewMode, bool bIsButton)
{
    bool bButtonChanged = false;
    bool bAxisChanged = false;
    bool bMainChanged = false;
    
    if (bIsButton)
    {
        if (NewMode != EInputMode::Unknown && NewMode != GetLastButtonInputMode(PlayerIndex))
        {
            if (PlayerIndex >= LastButtonPressByPlayer.Num())
                LastButtonPressByPlayer.SetNum(PlayerIndex + 1);
            
            LastButtonPressByPlayer[PlayerIndex] = NewMode;

            bButtonChanged = true;
        }
    }
    else
    {
        if (NewMode != EInputMode::Unknown && NewMode != GetLastAxisInputMode(PlayerIndex))
        {
            if (PlayerIndex >= LastAxisMoveByPlayer.Num())
                LastAxisMoveByPlayer.SetNum(PlayerIndex + 1);
            
            LastAxisMoveByPlayer[PlayerIndex] = NewMode;

            bAxisChanged = true;
        }
        
    }
    // Whether it's a button or not it can affect the main input mode
    if (NewMode != EInputMode::Unknown && NewMode != GetLastInputMode(PlayerIndex))
    {
        if (PlayerIndex >= LastInputModeByPlayer.Num())
            LastInputModeByPlayer.SetNum(PlayerIndex + 1);
        
        LastInputModeByPlayer[PlayerIndex] = NewMode;

        bMainChanged = true;
    }

    // Raise events at the end once all state has changed
    if (bButtonChanged)
    {
        // ReSharper disable once CppExpressionWithoutSideEffects
        OnButtonInputModeChanged.ExecuteIfBound(PlayerIndex, NewMode);
        //UE_LOG(LogStevesUEHelpers, Display, TEXT("Button mode for player %d changed: %s"), PlayerIndex, *UEnum::GetValueAsString(NewMode));
    }
    if (bAxisChanged)
    {
        // ReSharper disable once CppExpressionWithoutSideEffects
        OnAxisInputModeChanged.ExecuteIfBound(PlayerIndex, NewMode);
        //UE_LOG(LogStevesUEHelpers, Display, TEXT("Axis mode for player %d changed: %s"), PlayerIndex, *UEnum::GetValueAsString(NewMode));
    }
    if (bMainChanged)
    {
        // ReSharper disable once CppExpressionWithoutSideEffects
        OnInputModeChanged.ExecuteIfBound(PlayerIndex, NewMode);
        //UE_LOG(LogStevesUEHelpers, Display, TEXT("Input mode for player %d changed: %s"), PlayerIndex, *UEnum::GetValueAsString(NewMode));
    }

}

//PRAGMA_ENABLE_OPTIMIZATION