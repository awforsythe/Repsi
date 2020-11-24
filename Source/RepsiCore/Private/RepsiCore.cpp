#include "RepsiCore.h"
#include "Modules/ModuleManager.h"

#include "Log.h"

void FRepsiCore::StartupModule()
{
	UE_LOG(LogRepsiCore, Log, TEXT("RepsiCore module starting up"));
}

void FRepsiCore::ShutdownModule()
{
	UE_LOG(LogRepsiCore, Log, TEXT("RepsiCore module shutting down"));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FRepsiCore, RepsiCore, "RepsiCore");
