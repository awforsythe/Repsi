#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "TargetSphere.generated.h"

/** Simple test actor that can be shot by players. */
UCLASS()
class ATargetSphere : public AActor
{
	GENERATED_BODY()

public:
	/** Sphere mesh, with collision to block weapon traces. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UStaticMeshComponent* MeshComponent;

public:
	/** The color currently applied to the sphere mesh: changes server-side when the sphere is shot. */
	UPROPERTY(ReplicatedUsing=OnRep_Color, EditAnywhere, BlueprintReadWrite, Category="Target")
	FLinearColor Color;

	/** Duration of the animated transition that occurs in Tick when our Color value changes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Target")
	float ColorChangeDuration;

	/** MID applied to the mesh component, so its color can be changed at runtime. */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="Target")
	class UMaterialInstanceDynamic* MeshMID;

	/** Our previous color value, used to interpolate in Tick when our color value changes. */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="Target")
	FLinearColor PreviousColor;

	/** Local game time as of the last time our color was changed. */
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="Target")
	float LastColorChangeTime;

public:
	ATargetSphere(const FObjectInitializer& ObjectInitializer);
	virtual void PostInitializeComponents() override;
	virtual float InternalTakePointDamage(float Damage, struct FPointDamageEvent const& PointDamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	/** Updates the MID's Color parameter to reflect the current value of the Color property. */
	UFUNCTION() void OnRep_Color();
};
