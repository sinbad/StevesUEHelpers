// Original Copyright (c) Sam Bloomberg https://github.com/redxdev/UnrealRichTextDialogueBox (MIT License)
// Updates:
//   1. Fixed adding a spurious newline to single-line text
//   2. Expose line finished as event
//   3. Changed names of classes to indicate functionality better (typewriter not dialogue)
//   4. Update minimum desired size on calculate so that flexi boxes can start at the correct size
//      instead of growing when the newline is added

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/RichTextBlock.h"
#include "Framework/Text/RichTextLayoutMarshaller.h"
#include "Framework/Text/SlateTextLayout.h"
#include "TypewriterTextWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTypewriterLineFinished, class UTypewriterTextWidget*, Widget);
/**
 * A text block that exposes more information about text layout for typewriter widget.
 */
UCLASS()
class URichTextBlockForTypewriter : public URichTextBlock
{
	GENERATED_BODY()

public:
	FORCEINLINE TSharedPtr<FSlateTextLayout> GetTextLayout() const
	{
		return TextLayout;
	}

	FORCEINLINE TSharedPtr<FRichTextLayoutMarshaller> GetTextMarshaller() const
	{
		return TextMarshaller;
	}

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	TSharedPtr<FSlateTextLayout> TextLayout;
	TSharedPtr<FRichTextLayoutMarshaller> TextMarshaller;
};

UCLASS(Blueprintable)
class STEVESUEHELPERS_API UTypewriterTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTypewriterTextWidget(const FObjectInitializer& ObjectInitializer);

	/// Event called when a line has finished playing, whether on its own or when skipped to end
	UPROPERTY(BlueprintAssignable)
	FOnTypewriterLineFinished OnTypewriterLineFinished;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URichTextBlockForTypewriter* LineText;

	// The amount of time between printing individual letters (for the "typewriter" effect).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	float LetterPlayTime = 0.025f;

	// The amount of time to wait after finishing the line before actually marking it completed.
	// This helps prevent accidentally progressing dialogue on short lines.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	float EndHoldTime = 0.15f;

	/// Set Text immediately
	UFUNCTION(BlueprintCallable)
	void SetText(const FText& InText);

	/// Set Text immediately
	UFUNCTION(BlueprintCallable)
	FText GetText() const;


	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void PlayLine(const FText& InLine, float Speed = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void GetCurrentLine(FText& OutLine) const { OutLine = CurrentLine; }

	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	bool HasFinishedPlayingLine() const { return bHasFinishedPlaying; }

	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void SkipToLineEnd();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Typewriter")
	void OnPlayLetter();

	UFUNCTION(BlueprintImplementableEvent, Category = "Typewriter")
	void OnLineFinishedPlaying();

private:
	void PlayNextLetter();

	void CalculateWrappedString();
	FString CalculateSegments();

	UPROPERTY()
	FText CurrentLine;

	struct FTypewriterTextSegment
	{
		FString Text;
		FRunInfo RunInfo;
	};
	TArray<FTypewriterTextSegment> Segments;

	// The section of the text that's already been printed out and won't ever change.
	// This lets us cache some of the work we've already done. We can't cache absolutely
	// everything as the last few characters of a string may change if they're related to
	// a named run that hasn't been completed yet.
	FString CachedSegmentText;
	int32 CachedLetterIndex = 0;

	int32 CurrentSegmentIndex = 0;
	int32 CurrentLetterIndex = 0;
	int32 MaxLetterIndex = 0;
	int32 NumberOfLines = 0;
	float CombinedTextHeight = 0;

	uint32 bHasFinishedPlaying : 1;

	FTimerHandle LetterTimer;
};