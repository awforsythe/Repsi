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
	UPROPERTY(ReplicatedUsing=OnRep_Weapon, Transient, VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	class AWeapon* Weapon;

	/** How far into the scene we'll trace in order to figure out what the player is aiming at with their weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float AimTraceDistance;

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
	virtual void Tick(float DeltaSeconds) override;

	/** Server-only: updates the color applied to this pawn's mesh MID. */
	void AuthSetColor(const FLinearColor& InColor);

private:
	/** For client-side Pawns, ensures that the Weapon is attached to the WeaponHandle. */
	UFUNCTION()
	void OnRep_Weapon();

	/** Updates the MeshMID's color parameter to match our current Color property. */
	UFUNCTION()
	void OnRep_Color();

private:
	UFUNCTION() void OnFire();
	UFUNCTION() void OnMoveForward(float AxisValue);
	UFUNCTION() void OnMoveRight(float AxisValue);
	UFUNCTION() void OnMoveUp(float AxisValue);
	UFUNCTION() void OnLookRight(float AxisValue);
	UFUNCTION() void OnLookUp(float AxisValue);
	UFUNCTION() void OnLookRightRate(float AxisValue);
	UFUNCTION() void OnLookUpRate(float AxisValue);
};
