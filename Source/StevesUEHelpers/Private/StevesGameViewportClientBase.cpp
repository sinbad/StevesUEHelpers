#include "StevesGameViewportClientBase.h"


#include "Engine/Console.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"

void UStevesGameViewportClientBase::Init(FWorldContext& WorldContext, UGameInstance* OwningGameInstance,
    bool bCreateNewAudioDevice)
{
    Super::Init(WorldContext, OwningGameInstance, bCreateNewAudioDevice);

    bSuppressMouseCursor = false;
}

EMouseCursor::Type UStevesGameViewportClientBase::GetCursor(FViewport* InViewport, int32 X, int32 Y)
{
    if (FSlateApplication::Get().IsActive() && bSuppressMouseCursor)
        return EMouseCursor::None;

    return Super::GetCursor(InViewport, X, Y);
}

void UStevesGameViewportClientBase::SetSuppressMouseCursor(bool bSuppress)
{
    bSuppressMouseCursor = bSuppress;
    FSlateApplication::Get().OnCursorSet(); // necessary to make slate wake up
    
}
