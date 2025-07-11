// 

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "MultiSubtitleTextblock.generated.h"

/**
 * A text block that can show multiple lines of subtitles. The subtitles are
 * displayed in the text box with the most recent line first, and subtitles
 * time out after a configurable time once they stop being emitted by the
 * engine.
 * 
 * NOTE: The implementation of this class is arguably quite horrible, as
 * it relies on concatenating string versions of the subtitle texts. This is
 * *almost certainly* going to cause problems with i18n. If you use this
 * widget, expect it to not do the right thing with some languages!
 * Don't blame Steve - this is all Jeremiah Fieldhaven's fault.
 */
UCLASS()
class STEVESUEHELPERS_API UMultiSubtitleTextblock : public UTextBlock
{
	GENERATED_BODY()

public:
	virtual void BeginDestroy() override;

	/// The number of subtitles that may be shown at any given time.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 ConcurrentLines = 1;

	/// The length of time in seconds subtitles should remain in the block for.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaximumAge = 4;

protected:
	bool bSubscribed;

	void SetSubtitleText(const FText& Text);
	virtual TSharedRef<SWidget> RebuildWidget() override;


private:
	/** Update the subtitle history list to remove timed-out lines, and
	 * 	then set the contents of the text box to the new list of subtitles.
	 */
	void UpdateSubtitles();

	struct FSubtitleHistory
	{
		FString Subtitle;
		uint64 LastUpdate;

		// Less than operator needed by TArray<> to sort the subtitles in
		// order of update.
		const bool operator<(const FSubtitleHistory& rhs) const
		{
			// Use > here to sort in reverse. The or ensures stability if two
			// subtitles are playing simultaneously
			return this->LastUpdate > rhs.LastUpdate || this->Subtitle < rhs.Subtitle;
		}
	};

	// Handle for a timer that calls UpdateSubtitles() once per second
	FTimerHandle SubtitleUpdateTimer;

	// The list of subtitles to show in the widget
	TArray<FSubtitleHistory> Subtitles;
};
