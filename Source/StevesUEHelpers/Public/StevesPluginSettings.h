#pragma once

#include "CoreMinimal.h"

#include "StevesPluginSettings.generated.h"

/**
* Settings for the plug-in.
*/
UCLASS(config=Engine)
class STEVESUEHELPERS_API UStevesPluginSettings
	: public UObject
{
	GENERATED_BODY()

public:
	/// Which directories to search for Enhanced Input Actions when referenced just by name in e.g. Rich Text Decorator
	UPROPERTY(config, EditAnywhere, Category = StevesUEHelpers, meta = (DisplayName = "Directories to search for Enhanced Input Actions", RelativeToGameContentDir, LongPackageName))
	TArray<FDirectoryPath> EnhancedInputActionSearchDirectories;

	UStevesPluginSettings() {} 
	
};
