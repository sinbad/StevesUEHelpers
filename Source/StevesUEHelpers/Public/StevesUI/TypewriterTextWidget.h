﻿// Original Copyright (c) Sam Bloomberg https://github.com/redxdev/UnrealRichTextDialogueBox (MIT License)
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTypewriterLetterAdded, class UTypewriterTextWidget*, Widget, const FString&, Char);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTypewriterRunNameChanged, class UTypewriterTextWidget*, Widget, const FString&, NewRunName);
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

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	
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

	/// Event called when one or more new letters have been displayed
	UPROPERTY(BlueprintAssignable)
	FOnTypewriterLetterAdded OnTypewriterLetterAdded;

	/// Event called when the "run name" of the text changes aka the rich text style markup. Also called when reverts to default.
	UPROPERTY(BlueprintAssignable)
	FOnTypewriterRunNameChanged OnTypewriterRunNameChanged;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URichTextBlockForTypewriter* LineText;

	/// The amount of time between printing individual letters (for the "typewriter" effect).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	float LetterPlayTime = 0.025f;

	/// The amount of time to wait after finishing the line before actually marking it completed.
	/// This helps prevent accidentally progressing dialogue on short lines.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	float EndHoldTime = 0.15f;

	/// How long to pause at sentence terminators ('.', '!', '?') before proceeding (ignored if at end of text)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	float PauseTimeAtSentenceTerminators = 0.5f;

	/// How many lines of text at most to print at once.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typewriter")
	int MaxNumberOfLines = 3;

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
	bool HasMoreLineParts() const { return bHasMoreLineParts; }

	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void PlayNextLinePart(float Speed = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Typewriter")
	void SkipToLineEnd();

	/// Get the name of the current rich text run, if any
	const FString& GetCurrentRunName() const { return CurrentRunName; }

protected:
	virtual void NativeConstruct() override;

	/// Called when on or more letters are added, if subclasses want to override
	UFUNCTION(BlueprintImplementableEvent, Category = "Typewriter")
	void OnPlayLetter();

	UFUNCTION(BlueprintImplementableEvent, Category = "Typewriter")
	void OnLineFinishedPlaying();

	/// Name of the current rich text run, if any
	/// Can be used to identify the style of the most recently played letter
	UPROPERTY(BlueprintReadOnly, Category = "Typewriter")
	FString CurrentRunName;


private:
	void PlayNextLetter();
	static bool IsSentenceTerminator(TCHAR Letter);
	static bool IsClauseTerminator(TCHAR Letter);
	static int FindLastTerminator(const FString& CurrentLineString, int Count);

	int CalculateMaxLength();
	void CalculateWrappedString(const FString& CurrentLineString);
	FString CalculateSegments(FString* OutCurrentRunName);
	void StartPlayLine();

	UPROPERTY()
	FText CurrentLine;

	FString RemainingLinePart;

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
	uint32 bHasMoreLineParts : 1;

	FTimerHandle LetterTimer;
	float CurrentPlaySpeed = 1;
	float PauseTime = 0;
	bool bFirstPlayLine = true;
};