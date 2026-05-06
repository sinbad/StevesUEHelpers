// Copyright Steve Streeting 2020 onwards
// Released under the MIT license
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FStevesUEHelpersEdModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
