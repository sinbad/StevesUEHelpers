#include "StevesUEHelpersEd.h"
#include "StevesFixedDataTableCustomisationLayout.h"

#define LOCTEXT_NAMESPACE "FStevesUEHelpersEdModule"

void FStevesUEHelpersEdModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("StevesFixedDataTableRowHandle", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FStevesFixedDataTableCustomisationLayout::MakeInstance));
}

void FStevesUEHelpersEdModule::ShutdownModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout("StevesFixedDataTableRowHandle");
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FStevesUEHelpersEdModule, StevesUEHelpersEd)