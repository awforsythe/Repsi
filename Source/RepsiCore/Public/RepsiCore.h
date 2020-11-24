#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FRepsiCore : public IModuleInterface
{
public:
	static inline FRepsiCore& Get()
	{
		return FModuleManager::LoadModuleChecked<FRepsiCore>("RepsiCore");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("RepsiCore");
	}

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
