#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Weapon.generated.h"

USTRUCT(BlueprintType)
struct FWeaponFirePacket
{
	GENERATED_BODY()

	UPROPERTY()
	float ServerFireTime;
};

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
	/** How long we're required to wait between successive shots. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Firing")
	float FireCooldown;

	/** Game time when the weapon was last fired, for cooldown checks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Firing")
	float LastFireTime;

	/** Visual effect to play (at the muzzle) when the weapon is fired. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Firing")
	class UParticleSystem* FireEffect;

	/** Sound to play when the weapon is fired. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Firing")
	class USoundBase* FireSound;

	/** Sound to play in response to a Fire input when the weapon isn't yet ready to fire. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Firing")
	class USoundBase* DryFireSound;

	/** Replicated to non-owning clients: contains information about the last fire event that the server generated for this weapon. */
	UPROPERTY(ReplicatedUsing=OnRep_LastFirePacket, VisibleAnywhere, BlueprintReadOnly, Category="Firing|State")
	FWeaponFirePacket LastFirePacket;

public:
	/** How quickly the weapon will rotate to orient itself toward the point where the player is aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aiming")
	float AimInterpSpeed;

	/** Alternative rotation interp speed used when dropping the weapon (because the player is aiming at a point that's too close or is otherwise invalid). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aiming")
	float DropInterpSpeed;

	/** Local-space rotation that the weapon will adopt when it's not being aimed at a valid point in the world. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aiming")
	FRotator DropRotation;

	/** World-space location representing where the player is aiming the weapon. */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="Aiming|State")
	FVector AimLocation;

	/** Indicates whether AimLocation is a point we should actually aim the weapon at: if not, the weapon will drop down (into the DropRotation) until the aim location becomes valid again. */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="Aiming|State")
	bool bAimLocationIsValid;

public:
	AWeapon(const FObjectInitializer& ObjectInitializer);
	virtual void GatherCurrentMovement() override;
	virtual void Tick(float DeltaSeconds) override;

	void UpdateAimLocation(const FVector& InWorldAimLocation, const FVector& InViewAimLocation);
	void HandleFireInput();

public:
	UFUNCTION(Server, Reliable)
	void Server_TryFire();

	UFUNCTION()
	void OnRep_LastFirePacket();

private:
	void PlayFireEffects();
	void PlayUnableToFireEffects();
};
