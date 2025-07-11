//

#include "StevesUI/MultiSubtitleTextblock.h"
#include "Misc/DateTime.h"
#include "SubtitleManager.h"

TSharedRef<SWidget> UMultiSubtitleTextblock::RebuildWidget()
{
	auto Ret = Super::RebuildWidget();

	if (!bSubscribed)
	{
		if (auto SM = FSubtitleManager::GetSubtitleManager())
		{
			SM->OnSetSubtitleText().AddUObject(this, &UMultiSubtitleTextblock::SetSubtitleText);
			bSubscribed = true;
		}
	}

	if (ConcurrentLines > 1)
	{
		if (!SubtitleUpdateTimer.IsValid() && GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				SubtitleUpdateTimer, this, &UMultiSubtitleTextblock::UpdateSubtitles, 1, true);
		}

		if (Subtitles.Num() != ConcurrentLines)
		{
			Subtitles.Empty();
			Subtitles.Init({}, ConcurrentLines);
		}
	}

	return Ret;
}

void UMultiSubtitleTextblock::BeginDestroy()
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

void UMultiSubtitleTextblock::SetSubtitleText(const FText& InText)
{
	// If the number of concurrent lines is such that the widget only ever 
	// shows one subtitle at a time, just display it as-is
	if (ConcurrentLines < 2)
	{
		SetText(InText);
		return;
	}

	// When there is no sound playing that generates a subtitle, InText
	// will be empty - let UpdateSubtitles() handle timing out
	if (InText.IsEmptyOrWhitespace())
		return;

	bool bFoundMatch = false;
	FString InString = InText.ToString();
	uint64 Now = FDateTime::UtcNow().GetTicks();
	
	// Go through the displayed subtitle list looking for the current subtitle
	// so its update time can be bumped if it exists in the list.
	for (FSubtitleHistory& Entry : Subtitles)
	{
		if (Entry.Subtitle == InString)
		{
			bFoundMatch = true;
			Entry.LastUpdate = Now;
						
			break;
		}
	}

	// Subtitle isn't in the current list?
	if (!bFoundMatch)
	{
		// Try to locate an entry in the list that isn't in use already, and
		// update it to contain the current subtitle.
		for (FSubtitleHistory& Entry : Subtitles)
		{
			if (Entry.Subtitle.IsEmpty())
			{
				bFoundMatch = true;
				Entry.Subtitle = InString;
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

			Entry.Subtitle = InString;
			Entry.LastUpdate = Now;
		}
	}

	UpdateSubtitles();
}

void UMultiSubtitleTextblock::UpdateSubtitles()
{ 
	FString Contents;
	uint64 Now = FDateTime::UtcNow().GetTicks();

	Subtitles.Sort();

	for (FSubtitleHistory& Entry : Subtitles)
	{
		// Remove subtitles that have timed out
		if (!Entry.Subtitle.IsEmpty() &&
			(Now - Entry.LastUpdate) > (MaximumAge * ETimespan::TicksPerMillisecond * 1000)) 
		{
			Entry.Subtitle = "";
		}

		if (!Entry.Subtitle.IsEmpty())
		{
			Contents += Entry.Subtitle + "\n";
		}
	}

	SetText(FText::FromString(Contents));
}
