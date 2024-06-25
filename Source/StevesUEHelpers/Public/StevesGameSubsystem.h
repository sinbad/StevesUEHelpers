#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputCoreTypes.h"
#include "PaperSprite.h"
#include "Framework/Application/IInputProcessor.h"
#include "StevesHelperCommon.h"
#include "StevesTextureRenderTargetPool.h"
#include "StevesUI/FocusSystem.h"
#include "StevesUI/InputImage.h"
#include "StevesUI/UiTheme.h"
#include "Templates/TypeHash.h"

#include "StevesGameSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInputModeChanged, int, PlayerIndex, EInputMode, InputMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnhancedInputMappingsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWindowForegroundChanged, bool, bFocussed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnhancedInputActionTriggered, const UInputAction*, Action, ETriggerEvent, TriggeredEvent);

/// Entry point for all the top-level features of the helper system
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

    struct FEnhancedInputInterest
    {
        
        TWeakObjectPtr<const UInputAction> Action;
        ETriggerEvent Trigger;
        FEnhancedInputInterest(const UInputAction* TheAction, ETriggerEvent TheTriggerEvent) : Action(TheAction), Trigger(TheTriggerEvent) {}

        
        friend uint32 GetTypeHash(const FEnhancedInputInterest& Arg)
        {
            return HashCombine(GetTypeHash(Arg.Action), GetTypeHash(Arg.Trigger));
        }

        friend bool operator==(const FEnhancedInputInterest& Lhs, const FEnhancedInputInterest& RHS)
        {
            return Lhs.Action == RHS.Action
                && Lhs.Trigger == RHS.Trigger;
        }

        friend bool operator!=(const FEnhancedInputInterest& Lhs, const FEnhancedInputInterest& RHS)
        {
            return !(Lhs == RHS);
        }
    };

    TSet<FEnhancedInputInterest> RegisteredEnhancedInputActionInterests;

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
        TArray<EInputMode> LastButtonPressByPlayer;
        TArray<EInputMode> LastAxisMoveByPlayer;
        
        const EInputMode DefaultInputMode = EInputMode::Mouse;
        const EInputMode DefaultButtonInputMode = EInputMode::Keyboard;
        const EInputMode DefaultAxisInputMode = EInputMode::Mouse;
        const float MouseMoveThreshold = 1;
        const float GamepadAxisThreshold = 0.2;

        bool ShouldProcessInputEvents() const;
    public:
        /// Whether this detector should ignore events (e.g. because the application is in the background)
        bool bIgnoreEvents = false;

        /// Event raised when main input mode changes for any reason 
        FInternalInputModeChanged OnInputModeChanged;
        /// Event raised when button input mode changes only 
        FInternalInputModeChanged OnButtonInputModeChanged;
        /// Event raised when axis input mode changes only 
        FInternalInputModeChanged OnAxisInputModeChanged;


        FInputModeDetector();

        virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
        virtual bool
        HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;
        virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
        virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
        virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent,
            const FPointerEvent* InGestureEvent) override;

        /// Get the last input mode from any kind of input
        EInputMode GetLastInputMode(int PlayerIndex = 0);
        /// Get the last input mode from button inputs (ignores axis changes, good for detecting if keyboard or mouse buttons are being used)
        EInputMode GetLastButtonInputMode(int PlayerIndex = 0);
        /// Get the last input mode from axis movement (ignores button presses, good for detecting if keyboard or mouse axes are being used for motion)
        EInputMode GetLastAxisInputMode(int PlayerIndex = 0);

        // Needed but unused
        virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}

    protected:
        static bool IsAGamepadButton(const FKey& Key);
        void ProcessKeyOrButton(int PlayerIndex, FKey Key);
        void SetMode(int PlayerIndex, EInputMode NewMode, bool bIsButton);
    };

protected:
    TSharedPtr<FInputModeDetector> InputDetector;
    FFocusSystem FocusSystem;
    bool bCheckedViewportClient = false;

    FTimerHandle ForegroundCheckHandle;

    UPROPERTY(BlueprintReadOnly)
    bool bIsForeground = true;

    UPROPERTY(BlueprintReadWrite)
    UUiTheme* DefaultUiTheme;

    TArray<FStevesTextureRenderTargetPoolPtr> TextureRenderTargetPools;

    void CreateInputDetector();
    void DestroyInputDetector();
    void InitTheme();
    void InitForegroundCheck();
    void CheckForeground();


    // Called by detector
    void OnInputDetectorModeChanged(int PlayerIndex, EInputMode NewMode);
    void OnButtonInputDetectorModeChanged(int PlayerIndex, EInputMode NewMode);
    void OnAxisInputDetectorModeChanged(int PlayerIndex, EInputMode NewMode);


    TSoftObjectPtr<UDataTable> GetGamepadImages(int PlayerIndex, const UUiTheme* Theme);
    UPaperSprite* GetImageSpriteFromTable(const FKey& Key, const TSoftObjectPtr<UDataTable>& Asset);
    UFUNCTION()
    void EnhancedInputActionTriggered(const FInputActionInstance& InputActionInstance);

public:

    /// Event raised when main input mode changed between gamepad and keyboard / mouse (for any of axis / button events)
    UPROPERTY(BlueprintAssignable)
    FOnInputModeChanged OnInputModeChanged;
    
    /// Event raised when the last button input changed between gamepad / keyboard / mouse
    /// This can happen at a different time to OnInputModeChanged, e.g. if that was triggered by a mouse move, but the
    /// last button pressed was still keyboard, you'd get this event later
    UPROPERTY(BlueprintAssignable)
    FOnInputModeChanged OnButtonInputModeChanged;
    
    /// Event raised when the last axis input changed between gamepad / keyboard / mouse
    /// This can happen at a different time to OnInputModeChanged, e.g. if that was triggered by a button press, but the
    /// last axis moved was still mouse, you'd get this event later
    UPROPERTY(BlueprintAssignable)
    FOnInputModeChanged OnAxisInputModeChanged;

    /// Event raised justr after the Enhanced Input mappings have changed
    /// Right now, this has to be user-triggered via NotifyEnhancedInputMappingsChanged, because the Enhanced Input
    /// plugin provides NO events to monitor it (sigh)
    UPROPERTY(BlueprintAssignable)
    FOnEnhancedInputMappingsChanged OnEnhancedInputMappingsChanged;

    /// Event fired when an enhanced input event that an interest has previously been registered in triggers.
    /// Nothing will fire on this event unless you call RegisterInterestInEnhancedInputAction to listen for it.
    UPROPERTY(BlueprintAssignable)
    FOnEnhancedInputActionTriggered OnEnhancedInputActionTriggered;
    
    /// Event raised when the game window's foreground status changes
    UPROPERTY(BlueprintAssignable)
    FOnWindowForegroundChanged OnWindowForegroundChanged;

    /// Gets the device where the most recent input event of any kind happened
    UFUNCTION(BlueprintCallable)
    EInputMode GetLastInputModeUsed(int PlayerIndex = 0) const { return InputDetector->GetLastInputMode(PlayerIndex); }
    /// Gets the device where the most recent button press happened
    UFUNCTION(BlueprintCallable)
    EInputMode GetLastInputButtonPressed(int PlayerIndex = 0) const { return InputDetector->GetLastButtonInputMode(PlayerIndex); }
    /// Gets the device where the most recent axis move happened
    UFUNCTION(BlueprintCallable)
    EInputMode GetLastInputAxisMoved(int PlayerIndex = 0) const { return InputDetector->GetLastAxisInputMode(PlayerIndex); }
    
    UFUNCTION(BlueprintCallable)
    bool LastInputWasGamePad(int PlayerIndex = 0) const { return GetLastInputModeUsed(PlayerIndex) == EInputMode::Gamepad; }

    /// Gets the default UI theme object (defaults to our own)
    /// You can override this if you want
    UUiTheme* GetDefaultUiTheme() { return DefaultUiTheme; };

    /// Changes the default theme to a different one
    void SetDefaultUiTheme(UUiTheme* NewTheme) { DefaultUiTheme = NewTheme; }

    /// Get the global focus system
    FFocusSystem* GetFocusSystem();

    /// Return whether the game is currently in the foreground
    bool IsForeground() const { return bIsForeground; }

    /**
     * @brief Get an input button / key / axis image as a sprite based on any combination of action / axis binding or manual key
     * @param BindingType The type of input binding to look up 
     * @param ActionOrAxis The name of the action or axis, if BindingType is looking for that
     * @param Key The explicit key you want to display, if the BindingType is set to custom key
     * @param DevicePreference The order of preference for images where multiple devices have mappings. In the case of multiple mappings for the same device, the first one will be used.
     * @param PlayerIndex The player index to look up the binding for 
     * @param Theme Optional explicit theme, if blank use the default theme
     * @return 
     */
    UFUNCTION(BlueprintCallable)
    UPaperSprite* GetInputImageSprite(EInputBindingType BindingType,
                                      FName ActionOrAxis,
                                      FKey Key,
                                      EInputImageDevicePreference DevicePreference,
                                      int PlayerIndex = 0,
                                      const UUiTheme* Theme = nullptr);

    /**
     * @brief Get an input button / key / axis image as a sprite based on an enhanced input action
     * @param Action The input action 
     * @param DevicePreference The order of preference for images where multiple devices have mappings. In the case of multiple mappings for the same device, the first one will be used.
     * @param PlayerIdx The player index to look up the binding for 
     * @param PC The player controller to look up the binding for 
     * @param Theme Optional explicit theme, if blank use the default theme
     * @return 
     */
    UPaperSprite* GetInputImageSpriteFromEnhancedInputAction(UInputAction* Action,
                                                             EInputImageDevicePreference DevicePreference,
                                                             int PlayerIdx,
                                                             APlayerController* PC,
                                                             UUiTheme* Theme = nullptr);
    
    /**
    * @brief Get an input button / key image from an action
    * @param Name The name of the action
    * @param DevicePreference The order of preference for images where multiple devices have mappings. In the case of multiple mappings for the same device, the first one will be used.
    * @param PlayerIndex The player index to look up the binding for 
    * @param Theme Optional explicit theme, if blank use the default theme
    * @return 
    */
    UPaperSprite* GetInputImageSpriteFromAction(const FName& Name,
                                                EInputImageDevicePreference DevicePreference =
                                                    EInputImageDevicePreference::Gamepad_Keyboard_Mouse,
                                                int PlayerIndex = 0,
                                                const UUiTheme* Theme = nullptr);
    /**
    * @brief Get an input image from an axis
    * @param Name The name of the axis
    * @param DevicePreference The order of preference for images where multiple devices have mappings. In the case of multiple mappings for the same device, the first one will be used.
    * @param PlayerIndex The player index to look up the binding for 
    * @param Theme Optional explicit theme, if blank use the default theme
    * @return 
    */
    UPaperSprite* GetInputImageSpriteFromAxis(const FName& Name,
                                              EInputImageDevicePreference DevicePreference =
                                                  EInputImageDevicePreference::Gamepad_Mouse_Keyboard,
                                              int PlayerIndex = 0,
                                              const UUiTheme* Theme = nullptr);


    
    
    /**
    * @brief Get an input image for a specific key
    * @param Key The key to look up
    * @param Theme Optional explicit theme, if blank use the default theme
    * @return 
    */
    UPaperSprite* GetInputImageSpriteFromKey(const FKey& Key, int PlayerIndex = 0, const UUiTheme* Theme = nullptr);

    /**
     * @brief Set the content of a slate brush from an atlas (e.g. sprite)
     * @param Brush The brush to update
     * @param AtlasRegion Atlas to use as source e.g. a Sprite
     * @param bMatchSize Whether to resize the brush to match the atlas entry
     */
    static void SetBrushFromAtlas(FSlateBrush* Brush, TScriptInterface<ISlateTextureAtlasInterface> AtlasRegion,
                          bool bMatchSize);



    /**
    * Retrieve a pool of texture render targets. If a pool doesn't exist with the given name, it can be created.
    * @param Name Identifier for the pool. 
    * @param bAutoCreate 
    * @return The pool, or null if it doesn't exist and bAutoCreate is false
    */
    FStevesTextureRenderTargetPoolPtr GetTextureRenderTargetPool(FName Name, bool bAutoCreate = true);

    /**
     * Notify this subsystem that changes have been made to the Enhanced Input mappings, e.g. adding or removing a context.
     * Unfortunately, the Enhanced Input plugin currently provides NO WAY for us to monitor context changes automatically,
     * so we need the user to tell us when they make a change.
     * This call is however slightly delayed before being acted upon, because EI defers the rebuild of mappings until the next tick.
     */
    UFUNCTION(BlueprintCallable)
    void NotifyEnhancedInputMappingsChanged();

    /** Attempt to find an enhanced input action by name in the configured folders.
     */
    TSoftObjectPtr<UInputAction> FindEnhancedInputAction(const FString& Name);


    /// Register an interest in an enhanced input action. Calling this will result in OnEnhancedInputActionTriggered being called
    /// when this action is triggered.
    /// This is mainly for use in UI bindings. You only need to call it once for each UI-specific action.
    UFUNCTION(BlueprintCallable)
    void RegisterInterestInEnhancedInputAction(const UInputAction* Action, ETriggerEvent TriggerEvent);
};
