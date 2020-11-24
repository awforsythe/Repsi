#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "RepsiPawn.generated.h"

UCLASS()
class ARepsiPawn : public ACharacter
{
	GENERATED_BODY()

public:
	ARepsiPawn(const FObjectInitializer& ObjectInitializer);
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	UFUNCTION() void OnMoveForward(float AxisValue);
	UFUNCTION() void OnMoveRight(float AxisValue);
	UFUNCTION() void OnMoveUp(float AxisValue);
	UFUNCTION() void OnLookRight(float AxisValue);
	UFUNCTION() void OnLookUp(float AxisValue);
	UFUNCTION() void OnLookRightRate(float AxisValue);
	UFUNCTION() void OnLookUpRate(float AxisValue);
};
