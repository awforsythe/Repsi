#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "RepsiGameMode.generated.h"

UCLASS()
class ARepsiGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** A sequence of arbitrary color values that will be assigned to newly-spawned player pawns. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Players")
	TArray<FLinearColor> PlayerColors;

	/** Index into PlayerColors indicating the last color value we assigned to a pawn. */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="Players")
	int32 LastPlayerColorIndex;

public:
	ARepsiGameMode(const FObjectInitializer& ObjectInitializer);
	virtual void SetPlayerDefaults(class APawn* PlayerPawn) override;
};
