#pragma once

#include "CoreMinimal.h"
#include "StevesAssetHelpers.generated.h"

class UObjectLibrary;

/// Class to help out with asset related tasks, mostly C++ only but defined as a BPL in case that's useful later
UCLASS()
class STEVESUEHELPERS_API UStevesAssetHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Find the soft object paths of blueprints matching an object library definition, in a given set of paths.
	 * @param InPaths The asset paths to search. Must be game-directory form e.g. /Game/Data
	 * @param ObjectLibrary The object library which defines the (super)class which you're looking for. You should
	 * have created this similar to: `UObjectLibrary::CreateLibrary(UYourAssetClass::StaticClass(), true, GIsEditor && !IsRunningCommandlet());`
	 * @param OutSoftPaths Output array of soft object paths to the blueprints found. You can resolve these using their ResolveObject() function.
	 * @returns The number of blueprint assets found
	 */
	static int FindBlueprintSoftPaths(const TArray<FDirectoryPath>& InPaths, UObjectLibrary* ObjectLibrary, TArray<FSoftObjectPath>& OutSoftPaths);
	/**
	 * Find the soft object paths of blueprints matching an object library definition, in a given set of paths.
	 * @param InPaths The asset paths to search. Must be game-directory form e.g. /Game/Data
	 * @param ObjectLibrary The object library which defines the (super)class which you're looking for. You should
	 * have created this similar to: `UObjectLibrary::CreateLibrary(UYourAssetClass::StaticClass(), true, GIsEditor && !IsRunningCommandlet());`
	 * @param OutSoftPaths Output array of soft object paths to the blueprints found. You can resolve these using their ResolveObject() function.
	 * @returns The number of blueprint assets found
	 */
	static int FindBlueprintSoftPaths(const TArray<FString>& InPaths, UObjectLibrary* ObjectLibrary, TArray<FSoftObjectPath>& OutSoftPaths);

	/**
	 * Find a list of loaded classes for blueprints matching an object library definition, in a given set of paths.
	 * @param InPaths The asset paths to search. Must be game-directory form e.g. /Game/Data
	 * @param ObjectLibrary The object library which defines the (super)class which you're looking for. You should
	 * have created this similar to: `UObjectLibrary::CreateLibrary(UYourAssetClass::StaticClass(), true, GIsEditor && !IsRunningCommandlet());`
	 * @param OutClasses Output array of loaded UClass objects representing the blueprints found. 
	 * @returns The number of blueprint assets found
	 */
	static int FindBlueprintClasses(const TArray<FDirectoryPath>& InPaths, UObjectLibrary* ObjectLibrary, TArray<UClass*>& OutClasses);
	/**
	 * Find the soft object paths of blueprints matching an object library definition, in a given set of paths.
	 * @param InPaths The asset paths to search. Must be game-directory form e.g. /Game/Data
	 * @param ObjectLibrary The object library which defines the (super)class which you're looking for. You should
	 * have created this similar to: `UObjectLibrary::CreateLibrary(UYourAssetClass::StaticClass(), true, GIsEditor && !IsRunningCommandlet());`
	 * @param OutClasses Output array of loaded UClass objects representing the blueprints found. 
	 * @returns The number of blueprint assets found
	 */
	static int FindBlueprintClasses(const TArray<FString>& InPaths, UObjectLibrary* ObjectLibrary, TArray<UClass*>& OutClasses);

	
};
