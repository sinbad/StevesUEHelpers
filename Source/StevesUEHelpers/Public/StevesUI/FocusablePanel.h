#pragma once

#include "CoreMinimal.h"
#include "FocusableUserWidget.h"

#include "FocusablePanel.generated.h"

/// Base class for a UI Panel which has the concept of having focus, delegated to one of its children.
/// When told, it can initialise focus to a default widget. It can also remember which of its children
/// are currently focussed and restore that later.
/// Calling SetFocusProperly does the default behaviour of preferring previous but falling back on the default.
UCLASS()
class STEVESUEHELPERS_API UFocusablePanel : public UFocusableUserWidget
{
    GENERATED_BODY()


public:
    /// The name of the widget which will be initially focussed by default
    /// This is a name because we can't link directly at edit time
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
    FName InitialFocusWidgetName;

    // I'd love to make the above a drop-down but it's a lot of faff
    // See Engine\Source\Editor\UMGEditor\Private\Customizations\WidgetNavigationCustomization.cpp
    // Specifically OnGenerateWidgetList

    /**
     * @brief Set the current focus to the initial focus widget
     * @return Whether the focus was successfully set
     */
    UFUNCTION(BlueprintCallable, Category="Focus")
    bool SetFocusToInitialWidget();

    /**
     * @brief Get's the desired focus widget. Allows implementation in BP for dynamically generated menus
     * @return The initial focus widget
     */
    UFUNCTION(BlueprintNativeEvent, Category="Focus")
    UWidget* GetInitialFocusWidget();

	/**
     * @brief Set the desired initial focus widget. Call to set the initial widget dynamically without overriding GetInitialFocusWidget
     * @param NewInitialFocus The initial focus widget
     */
    UFUNCTION(BlueprintCallable, Category="Focus")
    void SetInitialFocusWidget(UWidget* NewInitialFocus);
    
    /**
     * @brief Try to restore focus to the previously focussed child
     * @return Whether the focus was successfully set
     */
    UFUNCTION(BlueprintCallable, Category="Focus")
    bool RestorePreviousFocus() const;

    /**
     * @brief Try to save the currently focussed child as something that can be restored later.
     * @return Whether focus was saved
     */
    UFUNCTION(BlueprintCallable, Category="Focus")
    bool SavePreviousFocus();

    
    /// When SetFocusProperly is called, either restores previous selection or gives it to the initial selection
    virtual void SetFocusProperly_Implementation() override;
protected:

    /// The widget that should get the focus on init if in keyboard / gamepad mode
    /// Looked up at runtime from the FName
    TWeakObjectPtr<UWidget> InitialFocusWidget;

    /// Previously focussed child which can be restored
    TWeakObjectPtr<UWidget> PreviousFocusWidget;

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;


};
