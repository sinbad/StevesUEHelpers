// 


#include "StevesUI/NamedOptionWidgetBase.h"

#include "StevesUI.h"

void UNamedOptionWidgetBase::ClearOptions()
{
	Super::ClearOptions();

	NameIndex.Reset();
}

int UNamedOptionWidgetBase::AddNamedOption(FName Name, FText Option)
{
	int Idx = Super::AddOption(Option);
	NameIndex.Add(Name, Idx);
	return Idx;
}

void UNamedOptionWidgetBase::SetOptions(const TArray<FText>& NewOptions, int NewSelectedIndex)
{
	Super::SetOptions(NewOptions, NewSelectedIndex);


	UE_LOG(LogStevesUI, Warning, TEXT("Using SetOptions on a Named Option Widget loses the name lookup index, consider using SetNamedOptions instead"))
	NameIndex.Reset();
}

void UNamedOptionWidgetBase::SetNamedOptions(const TMap<FName, FText>& InOptions,
                                             FName NewSelectedName)
{
	Options.Reset();
	NameIndex.Reset();
	for (const auto& Pair : InOptions)
	{
		const int Idx = Options.Add(Pair.Value);
		NameIndex.Add(Pair.Key, Idx);
	}
	SetSelectedName(NewSelectedName);
}

FName UNamedOptionWidgetBase::GetSelectedName() const
{
	return SelectedName;
}

int UNamedOptionWidgetBase::GetIndexForName(FName Name)
{
	if (auto pIdx = NameIndex.Find(Name))
	{
		return *pIdx;
	}
	return INDEX_NONE;
}

FName UNamedOptionWidgetBase::GetNameForIndex(int Index)
{
	if (auto pName = NameIndex.FindKey(Index))
	{
		return *pName;
	}
	return NAME_None;
}

void UNamedOptionWidgetBase::SetSelectedIndex(int NewIndex)
{
	// Update which name this is
	// Do this first so that the event called in the superclass has this data as well
	SelectedName = GetNameForIndex(NewIndex);
	
	Super::SetSelectedIndex(NewIndex);
}

void UNamedOptionWidgetBase::SetSelectedName(FName Name)
{
	SetSelectedIndex(GetIndexForName(Name));
}
