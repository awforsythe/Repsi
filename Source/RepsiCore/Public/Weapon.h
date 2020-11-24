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
	AWeapon(const FObjectInitializer& ObjectInitializer);
};
