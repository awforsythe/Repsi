#include "RepsiHUD.h"

#include "Engine/Canvas.h"

#include "RepsiPawn.h"

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

		// Draw a static crosshair in the center of the screen
		DrawCrosshair(Pawn->Color * 1.33f, 16.0f, 6.0f);
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
