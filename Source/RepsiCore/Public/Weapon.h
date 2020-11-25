#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Weapon.generated.h"

UCLASS()
class AWeapon : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UStaticMeshComponent* MeshComponent;

	/** Positioned at the end of the weapon, where line traces should originate from (and where muzzle flash effects etc. should be spawned).*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class USceneComponent* MuzzleHandle;

public:
	/** How quickly the weapon will rotate to orient itself toward the point where the player is aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float AimInterpSpeed;

	/** Alternative rotation interp speed used when dropping the weapon (because the player is aiming at a point that's too close or is otherwise invalid). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float DropInterpSpeed;

	/** Local-space rotation that the weapon will adopt when it's not being aimed at a valid point in the world. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	FRotator DropRotation;

	/** World-space location representing where the player is aiming the weapon. */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="Weapon|State")
	FVector AimLocation;

	/** Indicates whether AimLocation is a point we should actually aim the weapon at: if not, the weapon will drop down (into the DropRotation) until the aim location becomes valid again. */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="Weapon|State")
	bool bAimLocationIsValid;

public:
	AWeapon(const FObjectInitializer& ObjectInitializer);
	virtual void GatherCurrentMovement() override;
	virtual void Tick(float DeltaSeconds) override;

	void UpdateAimLocation(const FVector& InWorldAimLocation, const FVector& InViewAimLocation);
};
