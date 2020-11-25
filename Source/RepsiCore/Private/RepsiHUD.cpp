#include "RepsiHUD.h"

#include "Engine/Canvas.h"

#include "RepsiPawn.h"
#include "Weapon.h"

void ARepsiHUD::DrawHUD()
{
	Super::DrawHUD();

	// Pull some data from the local Pawn
	ARepsiPawn* Pawn = Cast<ARepsiPawn>(GetOwningPawn());
	if (Pawn && Canvas)
	{
		// Draw horizontal bars at the top and bottom of the screen, just to
		// indicate this player's color
		DrawPlayerColorBar(Pawn->Color, false, 4.0f);
		DrawPlayerColorBar(Pawn->Color, true, 8.0f);

		// If the local Pawn has a weapon, check whether the weapon's aim
		// location is valid, and animate the size of the cursor so that it's
		// "expanded" when not aimed downrange. Note that this approach of
		// having the HUD call directly into more and more classes could get
		// messy with a more complex project: in production you'd want to
		// consider defining a cleaner, more explicitly structured interface
		// between your UI and game objects (in addition to using UMG-authored
		// UUserWidgets instead of raw canvas drawing via AHUD)
		if (Pawn->Weapon)
		{
			const float TargetExpandWeight = Pawn->Weapon->bAimLocationIsValid ? 0.0f : 1.0f;
			CrosshairExpandWeight = FMath::FInterpTo(CrosshairExpandWeight, TargetExpandWeight, RenderDelta, 8.0f);
		}

		// Draw a static crosshair in the center of the screen
		const float CrosshairSize = FMath::Lerp(16.0f, 22.0f, CrosshairExpandWeight);
		const float CrosshairGapSize = FMath::Lerp(6.0f, 20.0f, CrosshairExpandWeight);
		DrawCrosshair(Pawn->Color * 1.33f, CrosshairSize, CrosshairGapSize);
	}
}

void ARepsiHUD::DrawPlayerColorBar(const FLinearColor& Color, bool bBottom, float Height)
{
	check(Canvas);

	const float OriginX = 0.0f;
	const float OriginY = bBottom ? Canvas->SizeY - Height : 0.0f;
	DrawRect(Color, OriginX, OriginY, Canvas->SizeX, Height);
}

void ARepsiHUD::DrawCrosshair(const FLinearColor& Color, float TotalSize, float GapSize)
{
	check(Canvas);

	const float CenterX = Canvas->SizeX * 0.5f;
	const float CenterY = Canvas->SizeY * 0.5f;

	const float ArmOffset = TotalSize * 0.5f;
	const float GapOffset = GapSize * 0.5f;
	const float Thickness = 2.0f;

	DrawLine(CenterX - ArmOffset, CenterY, CenterX - GapOffset, CenterY, Color, Thickness);
	DrawLine(CenterX + GapOffset, CenterY, CenterX + ArmOffset, CenterY, Color, Thickness);
	DrawLine(CenterX, CenterY - ArmOffset, CenterX, CenterY - GapOffset, Color, Thickness);
	DrawLine(CenterX, CenterY + GapOffset, CenterX, CenterY + ArmOffset, Color, Thickness);
}
