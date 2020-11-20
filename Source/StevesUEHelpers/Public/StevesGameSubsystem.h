#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputCoreTypes.h"
#include "PaperSprite.h"
#include "Framework/Application/IInputProcessor.h"
#include "StevesHelperCommon.h"
#include "StevesUI/FocusSystem.h"
#include "StevesUI/UiTheme.h"

#include "StevesGameSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInputModeChanged, int, PlayerIndex, EInputMode, InputMode);

UCLASS(Config=Game)
class STEVESUEHELPERS_API UStevesGameSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

protected:
    /// The default UiTheme path, the theme to use if controls don't specifically link to one.
    /// Customise this in DefaultGame.ini
    /// [/Script/StevesUEHelpers.StevesGameSubsystem]
    /// DefaultUiThemePath="/Game/Some/Other/UiTheme.UiTheme"
    /// Regardless, remember to register this file as a Primary Asset in Project Settings,
    /// as described in https://docs.unrealengine.com/en-US/Engine/Basics/AssetsAndPackages/AssetManagement/index.html
    /// so that it's included when packaging
    UPROPERTY(Config)
    FString DefaultUiThemePath;
    

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;


protected:
    DECLARE_DELEGATE_TwoParams(FInternalInputModeChanged, int /* PlayerIndex */, EInputMode)
    /**
     * We seem to need a separate non-UObject for an IInputProcessor, my attempt to combine the 2 fails.
     * So this class just acts as a safe relay between Slate and UInputHelper
     * 
     * This class should be registered as an input processor in order to capture all input events & detect
     * what kind of devices are being used. We can't use PlayerController to do this reliably because in UMG
     * mode, all the mouse move events are consumed by Slate and you never see them, so it's not possible to
     * detect when the user moved a mouse.
     *
     * This class should be instantiated and used from some UObject of your choice, e.g. your GameInstance class,
     * something like this:
     *
     *    InputDetector = MakeShareable(new FInputModeDetector());
     *    FSlateApplication::Get().RegisterInputPreProcessor(InputDetector);
     *    InputDetector->OnInputModeChanged.BindUObject(this, &UMyGameInstance::OnInputDetectorModeChanged);
     *
     *    Note how the OnInputModeChanged on this object is a simple delegate, not a dynamic multicast etc, because
     *    this is not a UObject. You should relay the input mode event changed through the owner if you want to distribute
     *    the information further.
     */
    class FInputModeDetector : public IInputProcessor, public TSharedFromThis<FInputModeDetector>
    {
    protected:
        TArray<EInputMode> LastInputModeByPlayer;
        
        const EInputMode DefaultInputMode = EInputMode::Mouse;
        const float MouseMoveThreshold = 1;
        const float GamepadAxisThreshold = 0.2;
    public:

        // Single delegate caller, owner should propagate if they want (this isn't a UObject)
        FInternalInputModeChanged OnInputModeChanged;


        FInputModeDetector();

        virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
        virtual bool
        HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;
        virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
        virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
        virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent,
            const FPointerEvent* InGestureEvent) override;

        EInputMode GetLastInputMode(int PlayerIndex = 0);

        // Needed but unused
        virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}

    protected:
        void ProcessKeyOrButton(int PlayerIndex, FKey Key);
        void SetMode(int PlayerIndex, EInputMode NewMode);
    };

protected:
    TSharedPtr<FInputModeDetector> InputDetector;
    FFocusSystem FocusSystem;

    UPROPERTY(BlueprintReadWrite)
    UUiTheme* DefaultUiTheme;

    void CreateInputDetector();
    void DestroyInputDetector();
    void InitTheme();

    // Called by detector
    void OnInputDetectorModeChanged(int PlayerIndex, EInputMode NewMode);

    UPaperSprite* GetImageSpriteFromTable(const FKey& Key, const TSoftObjectPtr<UDataTable>& Asset);
    
public:

    /// Event raised when input mode changed between gamepad and keyboard / mouse
    UPROPERTY(BlueprintAssignable)
    FOnInputModeChanged OnInputModeChanged;

    UFUNCTION(BlueprintCallable)
    EInputMode GetLastInputModeUsed(int PlayerIndex = 0) const { return InputDetector->GetLastInputMode(PlayerIndex); }
	
    UFUNCTION(BlueprintCallable)
    bool LastInputWasGamePad(int PlayerIndex = 0) const { return GetLastInputModeUsed(PlayerIndex) == EInputMode::Gamepad; }

    /// Gets the default UI theme object (defaults to our own)
    /// You can override this if you want
    UUiTheme* GetDefaultUiTheme() { return DefaultUiTheme; };

    /// Changes the default theme to a different one
    void SetDefaultUiTheme(UUiTheme* NewTheme) { DefaultUiTheme = NewTheme; }

    /// Get the global focus system
    FFocusSystem* GetFocusSystem();


    /**
     * @brief Get an input button / key / axis image as a sprite based on any combination of action / axis binding or manual key
     * @param BindingType The type of input binding to look up 
     * @param ActionOrAxis The name of the action or axis, if BindingType is looking for that
     * @param Key The explicit key you want to display, if the BindingType is set to custom key
     * @param PlayerIndex The player index to look up the binding for 
     * @param Theme Optional explicit theme, if blank use the default theme
     * @return 
     */
    UFUNCTION(BlueprintCallable)
    UPaperSprite* GetInputImageSprite(EInputBindingType BindingType, FName ActionOrAxis, FKey Key, int PlayerIndex = 0, const UUiTheme* Theme = nullptr);

    /**
    * @brief Get an input button / key image from an action
    * @param Name The name of the action
    * @param PlayerIndex The player index to look up the binding for 
    * @param Theme Optional explicit theme, if blank use the default theme
    * @return 
    */
    UPaperSprite* GetInputImageSpriteFromAction(const FName& Name, int PlayerIndex = 0, const UUiTheme* Theme = nullptr);
    /**
    * @brief Get an input image from an axis
    * @param Name The name of the axis
    * @param PlayerIndex The player index to look up the binding for 
    * @param Theme Optional explicit theme, if blank use the default theme
    * @return 
    */
    UPaperSprite* GetInputImageSpriteFromAxis(const FName& Name, int PlayerIndex = 0, const UUiTheme* Theme = nullptr);
    /**
    * @brief Get an input image for a specific key
    * @param Key The key to look up
    * @param Theme Optional explicit theme, if blank use the default theme
    * @return 
    */
    UPaperSprite* GetInputImageSpriteFromKey(const FKey& Key, const UUiTheme* Theme = nullptr);

};
