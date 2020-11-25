#include "RepsiGameMode.h"

#include "RepsiPlayerController.h"
#include "RepsiPawn.h"
#include "RepsiHUD.h"

ARepsiGameMode::ARepsiGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = ARepsiPlayerController::StaticClass();
	DefaultPawnClass = ARepsiPawn::StaticClass();
	HUDClass = ARepsiHUD::StaticClass();

	// Establish a sequence of arbitrary colors that we'll apply to each player
	// pawn, round-robin in the order in which they're spawned
	PlayerColors.Add(FLinearColor(0.30f, 0.02f, 0.02f));
	PlayerColors.Add(FLinearColor(0.02f, 0.30f, 0.02f));
	PlayerColors.Add(FLinearColor(0.02f, 0.02f, 0.30f));
	LastPlayerColorIndex = -1;
}

void ARepsiGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	// If we're initializing a newly-spawned player pawn, assign it a color
	ARepsiPawn* RepsiPawn = Cast<ARepsiPawn>(PlayerPawn);
	if (RepsiPawn)
	{
		// Use the next color in our sequence
		const int32 PlayerColorIndex = (LastPlayerColorIndex + 1) % PlayerColors.Num();
		if (PlayerColors.IsValidIndex(PlayerColorIndex))
		{
			// The AuthSetColor function is meant to be called exclusively on
			// the server. We know this code only runs with authority, because
			// the GameMode itself only exists on the server.
			RepsiPawn->AuthSetColor(PlayerColors[PlayerColorIndex]);
			LastPlayerColorIndex = PlayerColorIndex;
		}
	}
}
