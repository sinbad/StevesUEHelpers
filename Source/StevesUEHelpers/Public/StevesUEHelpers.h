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
	if (IsValid(WorldContext) && IsValid(WorldContext->GetGameInstance()))
	{
		return WorldContext->GetGameInstance()->GetSubsystem<UStevesGameSubsystem>();		
	}
		
	return nullptr;
}