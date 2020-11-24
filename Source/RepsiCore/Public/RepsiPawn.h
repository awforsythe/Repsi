#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "RepsiPawn.generated.h"

UCLASS()
class ARepsiPawn : public ACharacter
{
	GENERATED_BODY()

public:
	/** Scene component indicating where the pawn's Weapon should be attached. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class USceneComponent* WeaponHandle;

public:
	/** The weapon that this player is holding, if any. */
	UPROPERTY(Replicated, Transient, VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	class AWeapon* Weapon;

public:
	/** Material instance assigned to the character mesh, giving us control over the shader parameters at runtime. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Player")
	class UMaterialInstanceDynamic* MeshMID;

	/** An arbitrary color that identifies this player; assigned by the game mode on spawn. Controls the color of the mesh. */
	UPROPERTY(ReplicatedUsing=OnRep_Color, Transient, BlueprintReadOnly, Category="Player")
	FLinearColor Color;

public:
	ARepsiPawn(const FObjectInitializer& ObjectInitializer);
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Server-only: updates the color applied to this pawn's mesh MID. */
	void AuthSetColor(const FLinearColor& InColor);

private:
	/** Updates the MeshMID's color parameter to match our current Color property. */
	UFUNCTION()
	void OnRep_Color();

private:
	UFUNCTION() void OnMoveForward(float AxisValue);
	UFUNCTION() void OnMoveRight(float AxisValue);
	UFUNCTION() void OnMoveUp(float AxisValue);
	UFUNCTION() void OnLookRight(float AxisValue);
	UFUNCTION() void OnLookUp(float AxisValue);
	UFUNCTION() void OnLookRightRate(float AxisValue);
	UFUNCTION() void OnLookUpRate(float AxisValue);
};
