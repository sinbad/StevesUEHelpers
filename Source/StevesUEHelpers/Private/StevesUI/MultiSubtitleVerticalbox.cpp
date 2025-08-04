// 


#include "StevesUI/MultiSubtitleVerticalbox.h"

#include "SubtitleManager.h"
#include "TimerManager.h"
#include "Engine/Font.h"
#include "Kismet/GameplayStatics.h"


TSharedRef<SWidget> UMultiSubtitleVerticalbox::RebuildWidget()
{
	auto Ret = Super::RebuildWidget();

	if (!bSubscribed)
	{
		if (auto SM = FSubtitleManager::GetSubtitleManager())
		{
			SM->OnSetSubtitleText().AddUObject(this, &UMultiSubtitleVerticalbox::SetSubtitleText);
			bSubscribed = true;
		}
	}

	if (!SubtitleUpdateTimer.IsValid() && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			SubtitleUpdateTimer, this, &UMultiSubtitleVerticalbox::UpdateSubtitles, 1, true);
	}

	if (ConcurrentLines != TextBlocks.Num())
	{
		// If there aren't enough textblocks in the vertical box, add some
		while (TextBlocks.Num() < ConcurrentLines)
		{
			TextBlocks.Add(CreateTextBlock());
		}

		// If there are too many, remove them and mark them for cleanup
		while (TextBlocks.Num() > ConcurrentLines)
		{
			TObjectPtr<UTextBlock> Last = TextBlocks.Pop();
			
			if (Last != nullptr)
			{
				RemoveChild(Last);
			}
		}
	}

	// Recreate the subtitles list if needed
	if (ConcurrentLines != Subtitles.Num())
	{
		Subtitles.Empty();
		Subtitles.Init({}, ConcurrentLines);
	}
	
	return Ret;
}


void UMultiSubtitleVerticalbox::BeginDestroy()
{
	Super::BeginDestroy();

	if (bSubscribed)
	{
		if (auto SM = FSubtitleManager::GetSubtitleManager())
		{
			SM->OnSetSubtitleText().RemoveAll(this);
			bSubscribed = false;
		}
	}

	// Clear the timer if it has been set up
	if (SubtitleUpdateTimer.IsValid() && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SubtitleUpdateTimer);
	}
}


TObjectPtr<UTextBlock> UMultiSubtitleVerticalbox::CreateTextBlock()
{ 
	TObjectPtr<UTextBlock> NewBlock = NewObject<UTextBlock>(this);

	if (NewBlock != nullptr)
	{
		// Copy the style settings over
		NewBlock->SetColorAndOpacity(ColorAndOpacity);
		NewBlock->SetMinDesiredWidth(MinDesiredWidth);
		NewBlock->SetFont(Font);
		NewBlock->SetStrikeBrush(StrikeBrush);
		NewBlock->SetShadowOffset(ShadowOffset);
		NewBlock->SetShadowColorAndOpacity(ShadowColorAndOpacity);

		AddChildToVerticalBox(NewBlock);

		return NewBlock;
	}
	return nullptr; 
}


void UMultiSubtitleVerticalbox::SetSubtitleText(const FText& InText)
{
	// When there is no sound playing that generates a subtitle, InText
	// will be empty - let UpdateSubtitles() handle timing out
	if (InText.IsEmptyOrWhitespace())
		return;

	bool bFoundMatch = false;
	uint64 Now = FDateTime::UtcNow().GetTicks();

	// Go through the displayed subtitle list looking for the current subtitle
	// so its update time can be bumped if it exists in the list.
	for (FSubtitleHistory& Entry : Subtitles)
	{
		if (Entry.Subtitle.EqualTo(InText))
		{
			bFoundMatch = true;
			Entry.LastUpdate = Now;
			break;
		}
	}

	if (!bFoundMatch)
	{
		// Try to locate an entry in the list that isn't in use already, and
		// update it to contain the current subtitle.
		for (FSubtitleHistory& Entry : Subtitles)
		{
			if (Entry.Subtitle.IsEmpty())
			{
				bFoundMatch = true;
				Entry.Subtitle = InText;
				Entry.LastUpdate = Now;
				break;
			}
		}	

		// Still not found a place to put the new subtitle? Replace the oldest
		// subtitle in the list with the new one.
		if (!bFoundMatch)
		{
			Subtitles.Sort();
			FSubtitleHistory& Entry = Subtitles.Last();

			Entry.Subtitle = InText;
			Entry.LastUpdate = Now;
		}

		UpdateSubtitles();
	}
}


void UMultiSubtitleVerticalbox::UpdateSubtitles() 
{
	uint64 Now = FDateTime::UtcNow().GetTicks();

	// Handle timeout of subtitles that haven't been updated recently
	for (FSubtitleHistory& Entry : Subtitles)
	{
		if (!Entry.Subtitle.IsEmpty() &&
			(Now - Entry.LastUpdate) > (MaximumAge * ETimespan::TicksPerMillisecond * 1000))
		{
			Entry.Subtitle = FText::GetEmpty();
		}
	}

	Subtitles.Sort();

	// Update the list of textblocks with the subtitle texts
	for (int i = 0; i < TextBlocks.Num(); ++i)
	{
		if (TextBlocks[i] != nullptr && i < Subtitles.Num())
		{
			TextBlocks[i]->SetText(Subtitles[i].Subtitle);
		}
	}
}
