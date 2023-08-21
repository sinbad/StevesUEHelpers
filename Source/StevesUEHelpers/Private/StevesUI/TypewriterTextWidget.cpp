// Original Copyright (c) Sam Bloomberg https://github.com/redxdev/UnrealRichTextDialogueBox (MIT License)

#include "StevesUI/TypewriterTextWidget.h"
#include "Engine/Font.h"
#include "Styling/SlateStyle.h"
#include "Widgets/Text/SRichTextBlock.h"

//PRAGMA_DISABLE_OPTIMIZATION

void URichTextBlockForTypewriter::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	TextLayout.Reset();
	TextMarshaller.Reset();
}

TSharedRef<SWidget> URichTextBlockForTypewriter::RebuildWidget()
{
	// Copied from URichTextBlock::RebuildWidget
	UpdateStyleData();

	TArray< TSharedRef< class ITextDecorator > > CreatedDecorators;
	CreateDecorators(CreatedDecorators);

	TextMarshaller = FRichTextLayoutMarshaller::Create(CreateMarkupParser(), CreateMarkupWriter(), CreatedDecorators, StyleInstance.Get());

	MyRichTextBlock =
		SNew(SRichTextBlock)
		.TextStyle(bOverrideDefaultStyle ? &GetDefaultTextStyleOverride() : &GetDefaultTextStyle())
		.Marshaller(TextMarshaller)
		.CreateSlateTextLayout(
			FCreateSlateTextLayout::CreateWeakLambda(this, [this] (SWidget* InOwner, const FTextBlockStyle& InDefaultTextStyle) mutable
			{
				TextLayout = FSlateTextLayout::Create(InOwner, InDefaultTextStyle);
				return StaticCastSharedPtr<FSlateTextLayout>(TextLayout).ToSharedRef();
			}));

	return MyRichTextBlock.ToSharedRef();
}

UTypewriterTextWidget::UTypewriterTextWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bHasFinishedPlaying = true;
}

void UTypewriterTextWidget::SetText(const FText& InText)
{
	if (IsValid(LineText))
	{
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		TimerManager.ClearTimer(LetterTimer);
		
		LineText->SetText(InText);

		bHasFinishedPlaying = true;
	}
}

FText UTypewriterTextWidget::GetText() const
{
	if (IsValid(LineText))
	{
		return LineText->GetText();
	}

	return FText();
}

void UTypewriterTextWidget::PlayLine(const FText& InLine, float Speed)
{
	CurrentLine = InLine;
	RemainingLinePart = CurrentLine.ToString();
	PlayNextLinePart(Speed);
}

void UTypewriterTextWidget::PlayNextLinePart(float Speed)
{
	check(GetWorld());

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer(LetterTimer);

	CurrentRunName = "";
	CurrentLetterIndex = 0;
	CachedLetterIndex = 0;
	CurrentSegmentIndex = 0;
	MaxLetterIndex = 0;
	NumberOfLines = 0;
	CombinedTextHeight = 0;
	CurrentPlaySpeed = Speed;
	Segments.Empty();
	CachedSegmentText.Empty();

	if (RemainingLinePart.IsEmpty())
	{
		if (IsValid(LineText))
		{
			LineText->SetText(FText::GetEmpty());
		}

		bHasFinishedPlaying = true;
		OnTypewriterLineFinished.Broadcast(this);
		OnLineFinishedPlaying();

		SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		if (IsValid(LineText))
		{
			LineText->SetText(FText::GetEmpty());
		}

		bHasMoreLineParts = false;
		bHasFinishedPlaying = false;

		if (bFirstPlayLine)
		{
			// Delay the very first PlayLine after construction, CalculateWrappedString is not reliable until a couple
			// of UI geometry updates. At first the geometry is 0, then it's just wrong, and then finally it settles.
			FTimerHandle TempHandle;
			FTimerDelegate DelayDelegate;
			DelayDelegate.BindUObject(this, &ThisClass::StartPlayLine);
			TimerManager.SetTimer(TempHandle, DelayDelegate, 0.2f, false);
		}
		else
		{
			StartPlayLine();
		}
	}
}

void UTypewriterTextWidget::StartPlayLine()
{
	CalculateWrappedString(RemainingLinePart);

	if (MaxNumberOfLines > 0 && NumberOfLines > MaxNumberOfLines)
	{
		int MaxLength = CalculateMaxLength();
		int TerminatorIndex = FindLastTerminator(RemainingLinePart, MaxLength);
		int Length = TerminatorIndex + 1;
		const FString& FirstLinePart = RemainingLinePart.Left(Length);

		CalculateWrappedString(FirstLinePart);

		RemainingLinePart.RightChopInline(Length);
		bHasMoreLineParts = true;
	}
		
	FTimerDelegate Delegate;
	Delegate.BindUObject(this, &ThisClass::PlayNextLetter);

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(LetterTimer, Delegate, LetterPlayTime/CurrentPlaySpeed, true);

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	bFirstPlayLine = false;

	PlayNextLetter();
	
}

void UTypewriterTextWidget::SkipToLineEnd()
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer(LetterTimer);

	CurrentLetterIndex = MaxLetterIndex - 1;
	if (IsValid(LineText))
	{
		LineText->SetText(FText::FromString(CalculateSegments(nullptr)));
	}

	bHasFinishedPlaying = true;
	OnTypewriterLineFinished.Broadcast(this);
	OnLineFinishedPlaying();
}

void UTypewriterTextWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bFirstPlayLine = true;
}

void UTypewriterTextWidget::PlayNextLetter()
{
	// Incorporate pauses as a multiple of play timer (may not be exact but close enough)
	if (PauseTime > 0)
	{
		PauseTime -= LetterPlayTime/CurrentPlaySpeed;
		if (PauseTime > 0)
			return;
	}

	FString NewRunName;
	const FString WrappedString = CalculateSegments(&NewRunName);
	if (IsValid(LineText))
	{
		LineText->SetText(FText::FromString(WrappedString));
	}

	if (NewRunName != CurrentRunName)
	{
		CurrentRunName = NewRunName;
		OnTypewriterRunNameChanged.Broadcast(this, NewRunName);
	}

	OnPlayLetter();
	OnTypewriterLetterAdded.Broadcast(this, WrappedString.Right(1));

	// TODO: How do we keep indexing of text i18n-friendly?
	if (CurrentLetterIndex < MaxLetterIndex)
	{
		++CurrentLetterIndex;
	}
	else
	{
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		TimerManager.ClearTimer(LetterTimer);

		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &ThisClass::SkipToLineEnd);

		TimerManager.SetTimer(LetterTimer, Delegate, EndHoldTime, false);
	}
}

bool UTypewriterTextWidget::IsSentenceTerminator(TCHAR Letter) const
{
	int32 Unused;
	return SentenceTerminators.FindChar(Letter, Unused);
}

bool UTypewriterTextWidget::IsClauseTerminator(TCHAR Letter) const
{
	int32 Unused;
	return ClauseTerminators.FindChar(Letter, Unused);
}

int UTypewriterTextWidget::FindLastTerminator(const FString& CurrentLineString, int Count) const
{
	int TerminatorIndex = CurrentLineString.FindLastCharByPredicate([this](TCHAR Char) { return IsSentenceTerminator(Char); }, Count);
	if (TerminatorIndex != INDEX_NONE)
	{
		return TerminatorIndex;
	}

	TerminatorIndex = CurrentLineString.FindLastCharByPredicate([this](TCHAR Char) { return IsClauseTerminator(Char); }, Count);
	if (TerminatorIndex != INDEX_NONE)
	{
		return TerminatorIndex;
	}

	TerminatorIndex = CurrentLineString.FindLastCharByPredicate(FText::IsWhitespace, Count);
	if (TerminatorIndex != INDEX_NONE)
	{
		return TerminatorIndex;
	}

	return (Count - 1);
}

int UTypewriterTextWidget::CalculateMaxLength()
{
	int MaxLength = 0;
	int CurrentNumberOfLines = 1;
	for (int i = 0; i < Segments.Num(); i++)
	{
		const FTypewriterTextSegment& Segment = Segments[i];
		MaxLength += Segment.Text.Len();
		if (Segment.Text.Equals(FString(TEXT("\n"))))
		{
			CurrentNumberOfLines++;
			if (MaxNumberOfLines > 0 && CurrentNumberOfLines > MaxNumberOfLines)
			{
				break;
			}
		}
	}

	return MaxLength;
}

void UTypewriterTextWidget::CalculateWrappedString(const FString& CurrentLineString)
{
	// Rich Text views give you:
	// - A blank block at the start for some reason
	// - One block per line (newline characters stripped)
	//   - Split for different runs (decorators)
	// - The newlines we add are the only newlines in the output so that's the number of lines
	// If we've got here, that means the text isn't empty so 1 line at least
	NumberOfLines = 1;
	MaxLetterIndex = 0;
	CombinedTextHeight = 0;
	Segments.Empty();
	if (IsValid(LineText) && LineText->GetTextLayout().IsValid())
	{
		TSharedPtr<FSlateTextLayout> Layout = LineText->GetTextLayout();
		TSharedPtr<FRichTextLayoutMarshaller> Marshaller = LineText->GetTextMarshaller();

		const FGeometry& TextBoxGeometry = LineText->GetCachedGeometry();
		const FVector2D TextBoxSize = TextBoxGeometry.GetLocalSize();

		Layout->ClearLines();
		Layout->SetWrappingWidth(TextBoxSize.X);
		Marshaller->SetText(CurrentLineString, *Layout.Get());
		Layout->UpdateLayout();

		bool bHasWrittenText = false;
		auto Views = Layout->GetLineViews();
		for (int v = 0; v < Views.Num(); ++v)
		{
			const FTextLayout::FLineView& View = Views[v];

			for (int b = 0; b < View.Blocks.Num(); ++b)
			{
				TSharedRef<ILayoutBlock> Block = View.Blocks[b];
				TSharedRef<IRun> Run = Block->GetRun();

				FTypewriterTextSegment Segment;
				Run->AppendTextTo(Segment.Text, Block->GetTextRange());

				// HACK: For some reason image decorators (and possibly other decorators that don't
				// have actual text inside them) result in the run containing a zero width space instead of
				// nothing. This messes up our checks for whether the text is empty or not, which doesn't
				// have an effect on image decorators but might cause issues for other custom ones.
				// Instead of emptying text, which might have some unknown effect, just mark it as empty 
				const bool bTextIsEmpty = Segment.Text.IsEmpty() ||
					(Segment.Text.Len() == 1 && Segment.Text[0] == 0x200B);
				const int TextLen = bTextIsEmpty ? 0 : Segment.Text.Len();
				const bool bRunIsEmpty = Segment.RunInfo.Name.IsEmpty();

				Segment.RunInfo = Run->GetRunInfo();
				Segments.Add(Segment);

				// A segment with a named run should still take up time for the typewriter effect.
				MaxLetterIndex += FMath::Max(TextLen, Segment.RunInfo.Name.IsEmpty() ? 0 : 1);

				if (!bTextIsEmpty || !bRunIsEmpty)
				{
					bHasWrittenText = true;
				}
			}

			if (bHasWrittenText)
			{
				CombinedTextHeight += View.TextHeight;
			}
			// Add check for an unnecessary newline after ever line even if there's nothing else to do, otherwise
			// we end up inserting a newline after a simple single line of text
			const bool bHasMoreText = v < (Views.Num() - 1);
			if (bHasWrittenText && bHasMoreText)
			{
				Segments.Add(FTypewriterTextSegment{TEXT("\n")});
				++NumberOfLines;
				++MaxLetterIndex;
			}
		}

		Layout->SetWrappingWidth(0);

		// Set the desired vertical size so that we're already the size we need to accommodate all lines
		// Without this, a flexible box size will grow as lines are added
		FVector2D Sz = GetMinimumDesiredSize();
		Sz.Y = CombinedTextHeight;
		SetMinimumDesiredSize(Sz);
		
		LineText->SetText(LineText->GetText());
	}
	else
	{
		Segments.Add(FTypewriterTextSegment{CurrentLineString});
		MaxLetterIndex = Segments[0].Text.Len();
	}

}

FString UTypewriterTextWidget::CalculateSegments(FString* OutCurrentRunName)
{
	FString Result = CachedSegmentText;

	int32 Idx = CachedLetterIndex;
	while (Idx <= CurrentLetterIndex && CurrentSegmentIndex < Segments.Num())
	{
		const FTypewriterTextSegment& Segment = Segments[CurrentSegmentIndex];
		if (!Segment.RunInfo.Name.IsEmpty())
		{
			Result += FString::Printf(TEXT("<%s"), *Segment.RunInfo.Name);

			if (Segment.RunInfo.MetaData.Num() > 0)
			{
				for (const TTuple<FString, FString>& MetaData : Segment.RunInfo.MetaData)
				{
					Result += FString::Printf(TEXT(" %s=\"%s\""), *MetaData.Key, *MetaData.Value);
				}
			}

			if (Segment.Text.IsEmpty())
			{
				Result += TEXT("/>");
				++Idx; // This still takes up an index for the typewriter effect.
			}
			else
			{
				Result += TEXT(">");
			}
		}

		bool bIsSegmentComplete = true;
		if (!Segment.Text.IsEmpty())
		{
			int32 LettersLeft = CurrentLetterIndex - Idx + 1;
			bIsSegmentComplete = LettersLeft >= Segment.Text.Len();
			LettersLeft = FMath::Min(LettersLeft, Segment.Text.Len());
			Idx += LettersLeft;

			Result += Segment.Text.Mid(0, LettersLeft);

			// Add pause for sentence ends
			if (Result.Len() > 0 &&
				IsSentenceTerminator(Result[Result.Len() - 1]) &&
				CurrentLetterIndex < MaxLetterIndex - 1) // Don't pause on the last letter, that's the end pause's job
			{
				// Look ahead to make sure we only pause on LAST sentence terminator in a chain of them,
				// and also optionally not if there isn't whitespace after (e.g. to not pause on ".txt")
				if (LettersLeft < Segment.Text.Len())
				{
					if (!IsSentenceTerminator(Segment.Text[LettersLeft]) &&
						(!bPauseOnlyIfWhitespaceFollowsSentenceTerminator || FText::IsWhitespace(Segment.Text[LettersLeft])))
					{
						PauseTime = PauseTimeAtSentenceTerminators;
					}
				}
			}

			if (!Segment.RunInfo.Name.IsEmpty())
			{
				Result += TEXT("</>");
			}
			if (OutCurrentRunName)
			{
				*OutCurrentRunName = Segment.RunInfo.Name;
			}
			
		}

		if (bIsSegmentComplete)
		{
			CachedLetterIndex = Idx;
			CachedSegmentText = Result;
			++CurrentSegmentIndex;
		}
		else
		{
			break;
		}
	}
	
	return Result;
}

//PRAGMA_ENABLE_OPTIMIZATION