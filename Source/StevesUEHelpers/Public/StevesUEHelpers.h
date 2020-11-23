#pragma once

#include "CoreMinimal.h"


#include "StevesGameSubsystem.h"
#include "Modules/ModuleManager.h"
#include "Engine/World.h"

DECLARE_LOG_CATEGORY_EXTERN(LogStevesUEHelpers, Verbose, Verbose);

class FStevesUEHelpers : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

inline UStevesGameSubsystem* GetStevesGameSubsystem(UWorld* WorldContext)
{
	if (IsValid(WorldContext) && WorldContext->IsGameWorld())
	{
		auto GI = WorldContext->GetGameInstance();
		if (IsValid(GI))
			return GI->GetSubsystem<UStevesGameSubsystem>();		
	}
		
	return nullptr;
}