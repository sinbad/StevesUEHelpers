// Copyright Steve Streeting 2020 onwards
// Released under the MIT license
#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlockDecorator.h"

#include "RichTextBlockInputImageDecorator.generated.h"

class UUiTheme;

UCLASS()
class STEVESUEHELPERS_API URichTextBlockInputImageDecorator : public URichTextBlockDecorator
{
    GENERATED_BODY()
  
public:

    virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;

};
