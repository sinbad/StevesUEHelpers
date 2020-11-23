#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlockDecorator.h"

#include "RichTextBlockInputImageDecorator.generated.h"

class UUiTheme;

UCLASS()
class STEVESUEHELPERS_API URichTextBlockInputImageDecorator : public URichTextBlockDecorator
{
    GENERATED_BODY()

protected:
    /// Custom theme to use for this input image set; if not supplied will use UStevesGameSubsystem::DefaultUiTheme
    UPROPERTY(EditAnywhere, BlueprintReadOnly)   
    UUiTheme* CustomTheme;
    
public:

    virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;

    virtual UUiTheme* GetCustomTheme() const {  return CustomTheme; } 
};
