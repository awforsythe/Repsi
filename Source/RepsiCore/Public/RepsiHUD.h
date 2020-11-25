#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "RepsiHUD.generated.h"

UCLASS()
class ARepsiHUD : public AHUD
{
	GENERATED_BODY()

public:
	/** Value between 0 and 1 indicating whether the crosshair is expanded to show that the player's aim location isn't valid (i.e. is too close to fire at). */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="HUD")
	float CrosshairExpandWeight;

public:
	virtual void DrawHUD() override;

private:
	void DrawPlayerColorBar(const FLinearColor& Color, bool bBottom, float Height);
	void DrawCrosshair(const FLinearColor& Color, float TotalSize, float GapSize);
};
