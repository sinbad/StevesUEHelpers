// 


#include "StevesUI/SubtitleTextblock.h"

#include "SubtitleManager.h"

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
	SetText(InText);
}
