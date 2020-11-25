#include "RepsiPawn.h"

#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Weapon.h"

ARepsiPawn::ARepsiPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Get references to default assets: hardcoding asset references in C++ will
	// cause those assets to be always loaded, but for the player pawn in this
	// little demo project, that's fine
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("SkeletalMesh'/Engine/EngineMeshes/SkeletalCube.SkeletalCube'"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("Material'/Game/Assets/Player/M_PlayerCube.M_PlayerCube'"));

	// Move the camera down to account for the smaller mesh height, and have
	// the pawn rotate to match the player's view rotation in pitch as well as
	// yaw (since the player can fly, it's not fixed to the ground plane)
	BaseEyeHeight = 18.0f;
	bUseControllerRotationPitch = true;

	// Establish a max distance for our aim traces
	AimTraceDistance = 3000.0f;

	// Make the collision capsule small enough to just cover our mesh. Note that
	// these component getter functions would only return null if we were to
	// explicit prevent ACharacter from creating those components (by calling
	// `DoNotCreateDefaultSubobject(ACharacter::MeshComponentName)` etc. on the
	// ObjectInitializer before passing it in to the Super constructor).
	UCapsuleComponent* CollisionComponent = GetCapsuleComponent();
	if (CollisionComponent)
	{
		CollisionComponent->SetCapsuleSize(32.0f, 40.0f, false);
	}

	// Initialize the mesh component and scale/move it slightly. Ordinarily it'd
	// make more sense to extend our Pawn class with a Blueprint so we could
	// fiddle with these values more intuitively; we're just keeping it in C++
	// for this demo project to keep things simple
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent)
	{
		MeshComponent->SetSkeletalMesh(MeshFinder.Object);
		MeshComponent->SetMaterial(0, MaterialFinder.Object);
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -22.0f));
		MeshComponent->SetRelativeScale3D(FVector(2.0f));
	}

	// Change the default CharacterMovementComponent behavior: we want our
	// player pawns to be able to fly around freely, and we want them to start
	// and stop on a dime
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->DefaultLandMovementMode = MOVE_Flying;
		MovementComponent->MaxAcceleration = 5000.0f;
		MovementComponent->MaxFlySpeed = 800.0f;
		MovementComponent->BrakingDecelerationFlying = 5000.0f;
	}

	// Create an additional SceneComponent and position it where we want the
	// root of the Weapon actor to be attached. We need to attach this to the
	// character mesh (rather than the root of the actor) in order for the
	// weapon to move smoothly along with the character (inheriting its client-
	// side prediction and interpolation), which means we need to do some wonky
	// stuff with transforms since we're just bashing temp assets together.
	WeaponHandle = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("WeaponHandle"));
	WeaponHandle->SetUsingAbsoluteScale(true);
	WeaponHandle->SetupAttachment(MeshComponent ? MeshComponent : RootComponent);
	WeaponHandle->SetRelativeLocation(FVector(15.0f, 8.0f, 6.0f));

}

void ARepsiPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// If we're running on the server, spawn a Weapon actor, attach it to the
	// WeaponHandle, and make sure we're its owner
	if (HasAuthority())
	{
		// Ensure that we're the Owner, both so that bNetUseOwnerRelevancy works
		// as expected on the Weapon, and so that the Weapon can trace its
		// ownership back to a PlayerController that possesses this Pawn
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Spawn the Weapon - since AWeapon is replicated, the newly-spawned
		// actor will be replicated to all clients where this pawn is relevant.
		// Our Pawn's Weapon property is replicated, so that reference will be
		// replicated to any client-side Pawns as well.
		const FVector SpawnLocation = WeaponHandle->GetComponentLocation();
		const FRotator SpawnRotation = WeaponHandle->GetComponentRotation();
		Weapon = GetWorld()->SpawnActor<AWeapon>(SpawnLocation, SpawnRotation, SpawnInfo);

		// Attach the weapon to the Pawn's WeaponHandle and add a tick
		// dependency to make sure that this Pawn will always tick before the
		// associated Weapon
		OnRep_Weapon();
	}

	// Create a dynamic material instance so we can change the color of our
	// cube on the fly. Again, this is the sort of thing that's better done in
	// Blueprints, but we're handling it here for simplicity.
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent)
	{
		MeshMID = MeshComponent->CreateDynamicMaterialInstance(0);
	}
}

void ARepsiPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// Bind movement inputs (mostly parroted from DefaultPawn.cpp)
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ARepsiPawn::OnMoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ARepsiPawn::OnMoveRight);
	PlayerInputComponent->BindAxis(TEXT("MoveUp"), this, &ARepsiPawn::OnMoveUp);
	PlayerInputComponent->BindAxis(TEXT("LookRight"), this, &ARepsiPawn::OnLookRight);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ARepsiPawn::OnLookUp);
	PlayerInputComponent->BindAxis(TEXT("LookRightRate"), this, &ARepsiPawn::OnLookRightRate);
	PlayerInputComponent->BindAxis(TEXT("LookUpRate"), this, &ARepsiPawn::OnLookUpRate);
}

void ARepsiPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// If we have a Weapon, run a per-frame line trace into the scene to
	// determine what we're aiming at - i.e., find the closest blocking geometry
	// that's centered in front of our view. Note that this happens
	// independently on the server and all clients: this trace isn't
	// server-authoritative; it's just cosmetic. In other words, the results of
	// this trace don't affect gameplay (unlike, for example, a trace that deals
	// damage when the weapon is fired); it's simply used to update the rotation
	// of the weapon to show what the player is aiming at. Therefore clients
	// don't need to wait on the server to tell them where to aim, they can just
	// figure out for themselves based on replicated information that they
	// already have (namely, the transform of the pawn).
	if (Weapon)
	{
		// Get our view location and forward vector: on clients, the mesh
		// component moves smoothly with client prediction and interpolation
		// (whereas the actor itself will have stuttery movement), so it's
		// preferable to get our forward vector from it
		const FVector ViewLocation = GetPawnViewLocation();
		const FTransform ViewTransform = GetMesh() ? GetMesh()->GetComponentTransform() : GetActorTransform();
		const FVector ViewForward = ViewTransform.GetUnitAxis(EAxis::X);

		// Prepare a line trace to find the first blocking primitive beneath the
		// center of our view
		const FVector& TraceStart = ViewLocation;
		const FVector TraceEnd = TraceStart + (ViewForward * AimTraceDistance);
		const FName ProfileName = UCollisionProfile::BlockAllDynamic_ProfileName;
		const FCollisionQueryParams QueryParams(TEXT("PlayerAim"), false, this);

		// Run the trace and convey the resulting location to the Weapon. If we
		// hit something, that's what we're aiming at; if we don't hit anything,
		// then just aim downrange toward the point where our trace stopped.
		FHitResult Hit;
		FVector WorldAimLocation;
		if (GetWorld()->LineTraceSingleByProfile(Hit, TraceStart, TraceEnd, ProfileName, QueryParams))
		{
			WorldAimLocation = Hit.ImpactPoint;
		}
		else
		{
			WorldAimLocation = TraceEnd;
		}

		// Transform our world location into view space so that our weapon can
		// make an informed decision about whether that location is valid for
		// it to actually point to (it may be too close, or at an extreme angle)
		const FVector ViewAimLocation = ViewTransform.InverseTransformPosition(WorldAimLocation);
		Weapon->UpdateAimLocation(WorldAimLocation, ViewAimLocation);
	}
}

void ARepsiPawn::AuthSetColor(const FLinearColor& InColor)
{
	// The "Auth" prefix is an unofficial convention used to indicate that a
	// function is only meant to be called with authority - i.e. it's not a
	// Server RPC, it's a function that should only be called by code that's
	// already running on the server
	checkf(HasAuthority(), TEXT("ARepsiPawn::AuthSetColor called on client"));

	// Update our replicated Color property: this change will propagate to
	// clients via replication, calling their OnRep_Color function as a notify
	Color = InColor;

	// Since we're the server, there's no replication to wait for: we want to
	// immediately update our MID to reflect the new Color value. Since that's
	// exactly what happens in our notify function, we can just call that
	// function directly.
	OnRep_Color();
}

void ARepsiPawn::OnRep_Weapon()
{
	if (Weapon)
	{
		Weapon->AttachToComponent(WeaponHandle, FAttachmentTransformRules::SnapToTargetIncludingScale);
		Weapon->AddTickPrerequisiteActor(this);
	}
}

void ARepsiPawn::OnRep_Color()
{
	// This notify function will only be called on clients, to let us know that
	// the Color property has changed on the server and has been updated
	// accordingly on the client
	if (MeshMID)
	{
		MeshMID->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

void ARepsiPawn::OnMoveForward(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
		const FVector ViewForward = FRotationMatrix(ControlRotation).GetScaledAxis(EAxis::X);
		AddMovementInput(ViewForward, AxisValue);
	}
}

void ARepsiPawn::OnMoveRight(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
		const FVector ViewRight = FRotationMatrix(ControlRotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(ViewRight, AxisValue);
	}
}

void ARepsiPawn::OnMoveUp(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		AddMovementInput(FVector::UpVector, AxisValue);
	}
}

void ARepsiPawn::OnLookRight(float AxisValue)
{
	AddControllerYawInput(AxisValue);
}

void ARepsiPawn::OnLookUp(float AxisValue)
{
	AddControllerPitchInput(AxisValue);
}

void ARepsiPawn::OnLookRightRate(float AxisValue)
{
	const float BaseRate = 45.0f;
	const float ScaledRate = BaseRate * AxisValue;
	const float DeltaTime = GetWorld()->GetDeltaSeconds() * CustomTimeDilation;
	AddControllerYawInput(ScaledRate * DeltaTime);
}

void ARepsiPawn::OnLookUpRate(float AxisValue)
{
	const float BaseRate = 45.0f;
	const float ScaledRate = BaseRate * AxisValue;
	const float DeltaTime = GetWorld()->GetDeltaSeconds() * CustomTimeDilation;
	AddControllerPitchInput(ScaledRate * DeltaTime);
}

void ARepsiPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARepsiPawn, Weapon);
	DOREPLIFETIME(ARepsiPawn, Color);
}
