#include "StevesUEHelpers.h"

#define LOCTEXT_NAMESPACE "FStevesUEHelpers"

DEFINE_LOG_CATEGORY(LogStevesUEHelpers)

void FStevesUEHelpers::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogStevesUEHelpers, Log, TEXT("Steve's UE Helpers Module Started"))
}

void FStevesUEHelpers::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogStevesUEHelpers, Log, TEXT("Steve's UE Helpers Module Stopped"))
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStevesUEHelpers, StevesUEHelpers)
