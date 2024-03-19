#include "StevesAssetHelpers.h"

#include "StevesUEHelpers.h"
#include "Engine/ObjectLibrary.h"

int UStevesAssetHelpers::FindBlueprintSoftPaths(const TArray<FDirectoryPath>& InPaths,
                                               UObjectLibrary* ObjectLibrary,
                                               TArray<FSoftObjectPath>& OutSoftPaths)
{
	// LoadBlueprintAssetDataFromPaths requires FString array, LoadBlueprintAssetDataFromPath just makes one array per call so no better
	// I like using FDirectoryPath for settings though since it enables browsing
	TArray<FString> StrPaths;
	for (auto& Dir : InPaths)
	{
		StrPaths.Add(Dir.Path);
	}
	return FindBlueprintSoftPaths(StrPaths, ObjectLibrary, OutSoftPaths);
}

int UStevesAssetHelpers::FindBlueprintSoftPaths(const TArray<FString>& InPaths,
	UObjectLibrary* ObjectLibrary,
	TArray<FSoftObjectPath>& OutSoftPaths)
{
	ObjectLibrary->LoadBlueprintAssetDataFromPaths(InPaths);
	ObjectLibrary->LoadAssetsFromAssetData();
	// Now they're all loaded, add them
	TArray<FAssetData> FoundAssets;
	ObjectLibrary->GetAssetDataList(FoundAssets);
	int Count = 0;
	for (auto& Asset : FoundAssets)
	{
		// Need to resolve BP generated class
		const FString GeneratedClassTag = Asset.GetTagValueRef<FString>(FBlueprintTags::GeneratedClassPath);
		if (GeneratedClassTag.IsEmpty())
		{
			UE_LOG(LogStevesUEHelpers, Warning, TEXT("Unable to find GeneratedClass value for asset %s"), *Asset.GetObjectPathString());
			continue;
		}
		FSoftObjectPath StringRef;
		StringRef.SetPath(FPackageName::ExportTextPathToObjectPath(GeneratedClassTag));
		OutSoftPaths.Add(StringRef);
		++Count;
	}

	// Don't use OutSoftPaths.Num() in case it wasn't empty to start with
	return Count;
}

int UStevesAssetHelpers::FindBlueprintClasses(const TArray<FDirectoryPath>& InPaths,
	UObjectLibrary* ObjectLibrary,
	TArray<UClass*>& OutClasses)
{
	// LoadBlueprintAssetDataFromPaths requires FString array, LoadBlueprintAssetDataFromPath just makes one array per call so no better
	// I like using FDirectoryPath for settings though since it enables browsing
	TArray<FString> StrPaths;
	for (auto& Dir : InPaths)
	{
		StrPaths.Add(Dir.Path);
	}
	return FindBlueprintClasses(StrPaths, ObjectLibrary, OutClasses);
}

int UStevesAssetHelpers::FindBlueprintClasses(const TArray<FString>& InPaths,
	UObjectLibrary* ObjectLibrary,
	TArray<UClass*>& OutClasses)
{
	TArray<FSoftObjectPath> SoftPaths;
	FindBlueprintSoftPaths(InPaths, ObjectLibrary, SoftPaths);
	int Count = 0;
	for (auto& SoftRef : SoftPaths)
	{
		if (UClass* TheClass = Cast<UClass>(SoftRef.ResolveObject()))
		{
			OutClasses.Add(TheClass);
			++Count;
		}
	}
	return Count;
	
}
