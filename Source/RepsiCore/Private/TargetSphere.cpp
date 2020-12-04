#include "TargetSphere.h"

#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/EngineTypes.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

#include "DamageType_WeaponFire.h"

ATargetSphere::ATargetSphere(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("Material'/Game/Assets/TargetSphere/M_TargetSphere.M_TargetSphere'"));

	// Enable replication, and set a relatively short cull distance for testing
	bReplicates = true;
	NetCullDistanceSquared = FMath::Square(1500.0f);

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
		Color = FLinearColor::MakeRandomColor();
		OnRep_Color();
		return Damage;
	}
	return Super::InternalTakePointDamage(Damage, PointDamageEvent, EventInstigator, DamageCauser);
}

void ATargetSphere::OnRep_Color()
{
	if (MeshMID)
	{
		MeshMID->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

void ATargetSphere::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATargetSphere, Color);
}
