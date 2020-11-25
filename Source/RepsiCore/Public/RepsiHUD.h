#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "RepsiHUD.generated.h"

UCLASS()
class ARepsiHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawPlayerColorBar(const FLinearColor& Color, bool bBottom, float Height);
	void DrawCrosshair(const FLinearColor& Color, float TotalSize, float GapSize);
};
