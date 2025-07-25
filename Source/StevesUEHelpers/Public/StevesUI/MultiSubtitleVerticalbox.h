#pragma once

#include "CoreMinimal.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "MultiSubtitleVerticalbox.generated.h"


/**
 * A text block that can show multiple lines of subtitles. The subtitles are
 * displayed in the text box with the most recent line first, and subtitles
 * time out after a configurable time once they stop being emitted by the
 * engine.
 */
UCLASS()
class STEVESUEHELPERS_API UMultiSubtitleVerticalbox : public UVerticalBox
{
	GENERATED_BODY()

public:
	/// The number of subtitles that may be shown at any given time.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0"), Category = "Subtitles")
	uint8 ConcurrentLines = 1;

	/// The length of time in seconds subtitles should remain in the block for.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = ( ClampMin = "1.0" ), Category = "Subtitles")
	float MaximumAge = 4;

	/** The color of the text shown in child text boxes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textbox Appearance")
	FSlateColor ColorAndOpacity = FLinearColor::White;
	;

	/** The minimum desired size for the text in child text boxes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textbox Appearance")
	float MinDesiredWidth;

	/** The font to render the text with in child text boxes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textbox Appearance")
	FSlateFontInfo Font;

	/** The brush to strike through text with in child text boxes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textbox Appearance")
	FSlateBrush StrikeBrush;

	/** The direction the shadow is cast in child text boxes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textbox Appearance")
	FVector2D ShadowOffset = FVector2D(1.0f, 1.0f);

	/** The color of the shadow in child text boxes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textbox Appearance", meta = (DisplayName = "Shadow Color"))
	FLinearColor ShadowColorAndOpacity = FLinearColor::Transparent;

	virtual void BeginDestroy() override;

protected:
	bool bSubscribed;

	void SetSubtitleText(const FText& Text);
	virtual TSharedRef<SWidget> RebuildWidget() override;

	struct FSubtitleHistory
	{
		FText  Subtitle;
		uint64 LastUpdate;

		// Less than operator needed by TArray<> to sort the subtitles in
		// order of update.
		const bool operator<(const FSubtitleHistory& rhs) const
		{
			// Use > here to sort in reverse. The or ensures stability if two
			// subtitles are playing simultaneously
			return !this -> Subtitle.IsEmpty() && (this->LastUpdate > rhs.LastUpdate || this->Subtitle.CompareTo(rhs.Subtitle) < 0);
		}
	};

	/** Create a new TextBlock widget and add it to the vertical box. This will
	 *  copy the 'Textbox Appearance' settings across into the new text block
	 *  before adding it to the vertical box.
	 * 
	 * @return A pointer to the newly created TextBlock, or nullptr on error
	 */
	TObjectPtr<UTextBlock> CreateTextBlock();

	/** Update the subtitle history list to remove timed-out lines, and
	 * 	then set the contents of the text box to the new list of subtitles.
	 */
	void UpdateSubtitles();

	// Handle for a timer that calls UpdateSubtitles() once per second
	FTimerHandle SubtitleUpdateTimer;

	// The list of text blocks in the widget
	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> TextBlocks;

	// The list of subtitles to show
	TArray<FSubtitleHistory> Subtitles;
};