#include "StevesGameSubsystem.h"
#include "StevesGameViewportClientBase.h"
#include "StevesUEHelpers.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"

//PRAGMA_DISABLE_OPTIMIZATION

void UStevesGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    CreateInputDetector();
    InitTheme();

    auto GI = GetGameInstance();        
    auto VC = Cast<UStevesGameViewportClientBase>(GI->GetGameViewportClient());
    if (!VC)
        UE_LOG(LogStevesUEHelpers, Warning, TEXT("Your GameViewportClient needs to be set to a subclass of UStevesGameViewportClientBase if you want full functionality!"))
    
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

void UStevesGameSubsystem::OnInputDetectorModeChanged(int PlayerIndex, EInputMode NewMode)
{
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

UStevesGameSubsystem::FInputModeDetector::FInputModeDetector()
{
    // 4 local players should be plenty usually (will expand if necessary)
    LastInputModeByPlayer.Init(EInputMode::Mouse, 4);
}

bool UStevesGameSubsystem::FInputModeDetector::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
    // Key down also registers for gamepad buttons
    ProcessKeyOrButton(InKeyEvent.GetUserIndex(), InKeyEvent.GetKey());

    // Don't consume
    return false;
}

bool UStevesGameSubsystem::FInputModeDetector::HandleAnalogInputEvent(FSlateApplication& SlateApp,
    const FAnalogInputEvent& InAnalogInputEvent)
{
    if (InAnalogInputEvent.GetAnalogValue() > GamepadAxisThreshold)
        SetMode(InAnalogInputEvent.GetUserIndex(), EInputMode::Gamepad);
    // Don't consume
    return false;
}

bool UStevesGameSubsystem::FInputModeDetector::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    FVector2D Dist = MouseEvent.GetScreenSpacePosition() - MouseEvent.GetLastScreenSpacePosition();
    if (FMath::Abs(Dist.X) > MouseMoveThreshold || FMath::Abs(Dist.Y) > MouseMoveThreshold)
    {
        SetMode(MouseEvent.GetUserIndex(), EInputMode::Mouse);
    }
    // Don't consume
    return false;
}

bool UStevesGameSubsystem::FInputModeDetector::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    // We don't care which button
    SetMode(MouseEvent.GetUserIndex(), EInputMode::Mouse);
    // Don't consume
    return false;
}

bool UStevesGameSubsystem::FInputModeDetector::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent,
    const FPointerEvent* InGestureEvent)
{
    SetMode(InWheelEvent.GetUserIndex(), EInputMode::Mouse);
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
        UE_LOG(LogTemp, Warning, TEXT("Input mode for player %d changed: %s"), PlayerIndex, *UEnum::GetValueAsString(NewMode));
    }
}

//PRAGMA_ENABLE_OPTIMIZATION