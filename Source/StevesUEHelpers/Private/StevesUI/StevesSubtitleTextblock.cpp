// 


#include "StevesUI/StevesSubtitleTextblock.h"

#include "SubtitleManager.h"
#include "Kismet/GameplayStatics.h"

TSharedRef<SWidget> UStevesSubtitleTextblock::RebuildWidget()
{
	auto Ret = Super::RebuildWidget();

	if (!bSubscribed)
	{
		if (auto SM = FSubtitleManager::GetSubtitleManager())
		{
			SM->OnSetSubtitleText().AddUObject(this, &UStevesSubtitleTextblock::SetSubtitleText);
			bSubscribed = true;
		}
	}
	
	return Ret;

}

void UStevesSubtitleTextblock::BeginDestroy()
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

void UStevesSubtitleTextblock::SetSubtitleText(const FText& InText)
{
	// We get called even when subtitles are disabled
	if (UGameplayStatics::AreSubtitlesEnabled() || InText.IsEmptyOrWhitespace())
	{
		SetText(InText);
	}
}
