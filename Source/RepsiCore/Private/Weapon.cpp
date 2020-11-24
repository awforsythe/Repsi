#include "Weapon.h"

#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AWeapon::AWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("Material'/Game/Assets/Weapon/M_Weapon.M_Weapon'"));

	// Make sure that weapons will be replicated as long as their owning Pawn
	// is replicated
	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	// Create a static mesh, as well as a SceneComponent at the location of the
	// muzzle (i.e. where line traces would originate from, and where effects
	// would spawn)
	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent"));

	MeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetStaticMesh(MeshFinder.Object);
	MeshComponent->SetMaterial(0, MaterialFinder.Object);
	MeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	MeshComponent->SetRelativeLocation(FVector(20.0f, 0.0f, 0.0f));
	MeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 45.0f));
	MeshComponent->SetRelativeScale3D(FVector(0.5f, 0.15f, 0.15f));

	MuzzleHandle = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("MuzzleHandle"));
	MuzzleHandle->SetupAttachment(RootComponent);
	MuzzleHandle->SetRelativeLocation(FVector(50.0f, 0.0f, 0.0f));
}
