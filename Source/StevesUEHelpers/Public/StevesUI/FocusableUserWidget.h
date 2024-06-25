#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FocusableUserWidget.generated.h"

// Hacky intermediate type for UUserWidget so that we can have focusable child widgets via SetFocusProperly
UCLASS()
class STEVESUEHELPERS_API UFocusableUserWidget : public UUserWidget
{
    GENERATED_BODY()

protected:

    /// If enabled, this widget will opt-in to the list of widgets which can be given focus
    /// automatically when another UFocusableUserWidget with focus is removed from the viewport.
    /// Useful for making sure something has the focus at all times without having to have cross-dependencies
    /// between UI parts, or events everywhere
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Focus")
    bool bEnableAutomaticFocus = false;

    /// If bEnableAutomaticFocus is enabled, then this is the focus priority associated with this widget.
    /// In the event that there is more than one auto focus widget available, the highest priority widget will win. 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Focus")
    int AutomaticFocusPriority = 0;

    virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

public:
    /// UWidget::SetFocus is not virtual FFS. This does the same as SetFocus by default but can be overridden,
    /// e.g. to delegate focus to specific children
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void SetFocusProperly();

    /// Whether this widget is *currently* requesting focus. Default is to use IsAutomaticFocusEnabled but subclasses
    /// may override this to be volatile
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool IsRequestingFocus() const;

    /// Tell this widget to take the focus if it desires to
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool TakeFocusIfDesired();

    virtual bool IsAutomaticFocusEnabled() const { return bEnableAutomaticFocus; }
    virtual int GetAutomaticFocusPriority() const { return AutomaticFocusPriority; }

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    
};
