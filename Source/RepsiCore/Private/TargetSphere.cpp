#include "TargetSphere.h"

#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/EngineTypes.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

#include "DamageType_WeaponFire.h"
#include "RepsiPawn.h"

ATargetSphere::ATargetSphere(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("Material'/Game/Assets/TargetSphere/M_TargetSphere.M_TargetSphere'"));

	// Enable replication, and set a relatively short cull distance for testing
	bReplicates = true;
	NetCullDistanceSquared = FMath::Square(1500.0f);

	// This actor will tick for a moment after it's shot, so it can animate its color
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	ColorChangeDuration = 0.333f;

	// Make our Actor damageable initially, so Weapon traces will deal damage
	SetCanBeDamaged(true);

	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

	MeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetStaticMesh(MeshFinder.Object);
	MeshComponent->SetMaterial(0, MaterialFinder.Object);
}

void ATargetSphere::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	MeshMID = MeshComponent->CreateDynamicMaterialInstance(0);
}

float ATargetSphere::InternalTakePointDamage(float Damage, FPointDamageEvent const& PointDamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// If shot with a Weapon, accept the damage event and randomize the sphere's Color
	if (PointDamageEvent.DamageTypeClass == UDamageType_WeaponFire::StaticClass())
	{
		const ARepsiPawn* Pawn = EventInstigator ? EventInstigator->GetPawn<ARepsiPawn>() : nullptr;
		if (Pawn)
		{
			Color = Pawn->Color;
			OnRep_Color();
		}
		return Damage;
	}
	return Super::InternalTakePointDamage(Damage, PointDamageEvent, EventInstigator, DamageCauser);
}

void ATargetSphere::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Compute our relative progress in the color change animation
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float ColorChangeElapsed = CurrentTime - LastColorChangeTime;
	const float ColorChangeAlpha = ColorChangeDuration < KINDA_SMALL_NUMBER ? 1.0f : FMath::Min(1.0f, ColorChangeElapsed / ColorChangeDuration);

	// Start interpolating toward a brighter version of new color, settling on
	// the unmodified color at the end
	const FLinearColor TargetColor = Color * FMath::Lerp(10.0f, 1.0f, ColorChangeAlpha);
	const FLinearColor NewColor = FLinearColor::LerpUsingHSV(PreviousColor, TargetColor, ColorChangeAlpha);
	if (MeshMID)
	{
		MeshMID->SetVectorParameterValue(TEXT("Color"), NewColor);
	}

	// Disable ticking once the animation is finished
	if (ColorChangeAlpha >= 1.0f)
	{
		SetActorTickEnabled(false);
	}
}

void ATargetSphere::OnRep_Color()
{
	if (MeshMID)
	{
		PreviousColor = MeshMID->K2_GetVectorParameterValue(TEXT("Color"));
	}
	LastColorChangeTime = GetWorld()->GetTimeSeconds();
	SetActorTickEnabled(true);
}

void ATargetSphere::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATargetSphere, Color);
}
