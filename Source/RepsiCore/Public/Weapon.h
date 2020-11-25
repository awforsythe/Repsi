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

	/** World-space location representing where the player is aiming the weapon. */
	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, Category="Weapon")
	FVector AimLocation;

public:
	AWeapon(const FObjectInitializer& ObjectInitializer);
	virtual void GatherCurrentMovement() override;
	virtual void Tick(float DeltaSeconds) override;
};
