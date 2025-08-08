#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "StevesPluginSettings.generated.h"

/**
* Settings for the plug-in.
*/
UCLASS(config=Engine, DefaultConfig)
class STEVESUEHELPERS_API UStevesPluginSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category = StevesUEHelpers, meta = (ToolTip = "Whether to automatically hide the mouse cursor and move it offscreen when using a gamepad"))
	bool bHideMouseWhenGamepadUsed = true;

	/// Which directories to search for Enhanced Input Actions when referenced just by name in e.g. Rich Text Decorator
	UPROPERTY(config, EditAnywhere, Category = StevesUEHelpers, meta = (DisplayName = "Directories to search for Enhanced Input Actions", RelativeToGameContentDir, LongPackageName))
	TArray<FDirectoryPath> EnhancedInputActionSearchDirectories;

	UStevesPluginSettings() {} 
	
};
