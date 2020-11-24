#include "RepsiGameMode.h"

#include "RepsiPlayerController.h"
#include "RepsiPawn.h"

ARepsiGameMode::ARepsiGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = ARepsiPlayerController::StaticClass();
	DefaultPawnClass = ARepsiPawn::StaticClass();
}
