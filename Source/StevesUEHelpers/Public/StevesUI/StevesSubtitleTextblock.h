// 

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "StevesSubtitleTextblock.generated.h"

/**
 * A simple Text Block that automatically displays subtitle text. Just drop it in and style it
 * how you want, subtitles will appear here when played, and the default canvas-rendered crappy
 * versions will be disabled. 
 */
UCLASS()
class STEVESUEHELPERS_API UStevesSubtitleTextblock : public UTextBlock
{
	GENERATED_BODY()

public:
	virtual void BeginDestroy() override;

protected:
	bool bSubscribed;

	void SetSubtitleText(const FText& Text);
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
