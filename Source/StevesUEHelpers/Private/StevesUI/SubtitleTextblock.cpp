// 


#include "StevesUI/SubtitleTextblock.h"

#include "SubtitleManager.h"
#include "Kismet/GameplayStatics.h"

TSharedRef<SWidget> USubtitleTextblock::RebuildWidget()
{
	auto Ret = Super::RebuildWidget();

	if (!bSubscribed)
	{
		if (auto SM = FSubtitleManager::GetSubtitleManager())
		{
			SM->OnSetSubtitleText().AddUObject(this, &USubtitleTextblock::SetSubtitleText);
			bSubscribed = true;
		}
	}
	
	return Ret;

}

void USubtitleTextblock::BeginDestroy()
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
}

void USubtitleTextblock::SetSubtitleText(const FText& InText)
{
	// We get called even when subtitles are disabled
	if (UGameplayStatics::AreSubtitlesEnabled() || InText.IsEmptyOrWhitespace())
	{
		SetText(InText);
	}
}
