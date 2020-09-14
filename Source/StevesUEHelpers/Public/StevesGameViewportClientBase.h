#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"

#include "StevesGameViewportClientBase.generated.h"

UCLASS()
class STEVESUEHELPERS_API UStevesGameViewportClientBase : public UGameViewportClient
{
    GENERATED_BODY()

    protected:
    bool bSuppressMouseCursor;
    
    public:


    virtual void Init(FWorldContext& WorldContext, UGameInstance* OwningGameInstance,
        bool bCreateNewAudioDevice) override;
    virtual EMouseCursor::Type GetCursor(FViewport* Viewport, int32 X, int32 Y) override;

    virtual void SetSuppressMouseCursor(bool bSuppress);
};
