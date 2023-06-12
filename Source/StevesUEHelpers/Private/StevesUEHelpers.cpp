#include "StevesUEHelpers.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "StevesPluginSettings.h"
#endif



#define LOCTEXT_NAMESPACE "FStevesUEHelpers"

DEFINE_LOG_CATEGORY(LogStevesUEHelpers)

void FStevesUEHelpers::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogStevesUEHelpers, Log, TEXT("Steve's UE Helpers Module Started"))

#if WITH_EDITOR
	// register settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "StevesUEHelpers",
			LOCTEXT("StevesUEHelpersSettingsName", "StevesUEHelpers"),
			LOCTEXT("StevesUEHelpersSettingsDescription", "Configure the helpers plug-in."),
			GetMutableDefault<UStevesPluginSettings>()
		);
	}
#endif
	
}

void FStevesUEHelpers::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogStevesUEHelpers, Log, TEXT("Steve's UE Helpers Module Stopped"))
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStevesUEHelpers, StevesUEHelpers)
