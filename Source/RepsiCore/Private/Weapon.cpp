#include "Weapon.h"

#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "Materials/MaterialInterface.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

#include "Log.h"
#include "DamageType_WeaponFire.h"

AWeapon::AWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("Material'/Game/Assets/Weapon/M_Weapon.M_Weapon'"));

	static ConstructorHelpers::FObjectFinder<UParticleSystem> FireEffectFinder(TEXT("ParticleSystem'/Game/Assets/Weapon/PS_MuzzleFlash.PS_MuzzleFlash'"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ImpactEffectFinder(TEXT("ParticleSystem'/Game/Assets/Weapon/PS_WeaponImpact.PS_WeaponImpact'"));

	static ConstructorHelpers::FObjectFinder<USoundBase> FireSoundFinder(TEXT("SoundCue'/Game/Audio/A_Weapon_Fire.A_Weapon_Fire'"));
	static ConstructorHelpers::FObjectFinder<USoundBase> DryFireSoundFinder(TEXT("SoundCue'/Game/Audio/A_Weapon_DryFire.A_Weapon_DryFire'"));
	static ConstructorHelpers::FObjectFinder<USoundBase> DamagingImpactSoundFinder(TEXT("SoundCue'/Game/Audio/A_WeaponImpact_Damaging.A_WeaponImpact_Damaging'"));
	static ConstructorHelpers::FObjectFinder<USoundBase> NonDamagingImpactSoundFinder(TEXT("SoundCue'/Game/Audio/A_WeaponImpact_NonDamaging.A_WeaponImpact_NonDamaging'"));

	PrimaryActorTick.bCanEverTick = true;

	// Define default cooldown/firing properties
	FireCooldown = 0.4f;
	LastFireTime = TNumericLimits<float>::Lowest();

	// Define default values for aiming properties
	AimInterpSpeed = 8.0f;
	DropInterpSpeed = 10.0f;
	DropRotation = FRotator(-30.0f, -80.0f, 0.0f);

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

	// Cache asset references for visual effects and audio
	FireEffect = FireEffectFinder.Object;
	ImpactEffect = ImpactEffectFinder.Object;
	FireSound = FireSoundFinder.Object;
	DryFireSound = DryFireSoundFinder.Object;
	DamagingImpactSound = DamagingImpactSoundFinder.Object;
	NonDamagingImpactSound = NonDamagingImpactSoundFinder.Object;
}

void AWeapon::GatherCurrentMovement()
{
}

void AWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bAimLocationIsValid)
	{
		const FVector AimDisplacement = AimLocation - GetActorLocation();
		const FVector AimDirection = AimDisplacement.GetSafeNormal();

		const FQuat TargetRotation = AimDirection.ToOrientationQuat();
		const FQuat NewRotation = FMath::QInterpTo(GetActorQuat(), TargetRotation, DeltaSeconds, AimInterpSpeed);
		SetActorRotation(NewRotation);
	}
	else
	{
		AActor* AttachParent = GetAttachParentActor();
		const FQuat TargetRotation = AttachParent ? AttachParent->GetActorTransform().TransformRotation(FQuat(DropRotation)) : FQuat(DropRotation);
		const FQuat NewRotation = FMath::QInterpTo(GetActorQuat(), TargetRotation, DeltaSeconds, DropInterpSpeed);
		SetActorRotation(NewRotation);
	}
}

void AWeapon::UpdateAimLocation(const FVector& InWorldAimLocation, const FVector& InViewAimLocation)
{
	AimLocation = InWorldAimLocation;
	bAimLocationIsValid = InViewAimLocation.X > MuzzleHandle->GetRelativeLocation().X;
}

void AWeapon::HandleFireInput()
{
	// This function is being called on the local client to let us know that
	// they've pressed the fire button. We'll do an initial client-side cooldown
	// check just to avoid spamming the server with unnecessary RPCs, but this
	// isn't an authoritative cooldown check: that's up to the server
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float ElapsedSinceLastFire = CurrentTime - LastFireTime;
	if (ElapsedSinceLastFire >= FireCooldown)
	{
		// Issue a Server RPC to notify the server that the client wants to
		// fire: the server will then make its own authoritative decision and
		// update game state accordingly. Note that if we're playing standalone,
		// this call just runs locally: it doesn't travel over the network, but
		// the end result is the same (and technically the separate client and
		// server cooldown checks are redundant in that case, but that's not
		// significant enough to warrant special-case logic for single-player)
		const FVector MuzzleLocation = MuzzleHandle->GetComponentLocation();
		const FVector Direction = MuzzleHandle->GetComponentQuat().Vector();
		Server_TryFire(MuzzleLocation, Direction);
		LastFireTime = CurrentTime;

		// If this weapon, belonging to the locally controlled player, has
		// authority, that means we're either running standalone or as a listen
		// server. In that case, Server_TryFire will execute right away, and it
		// will spawn cosmetic effects right away. But if we *don't* have
		// authority, we're running as a client, which means there's latency
		// between when the player fires the weapon and when the server decides
		// where the shot lands. So if we're a client, we want to spawn cosmetic
		// effects (particles, sound) right away, rather than waiting for the
		// server to tell us with 100% certainty that we've successfully fired).
		// This means our effects are non-authoritative (i.e. we're speculating
		// about what we hit and hoping that we're right, but the server has the
		// final say and might disagree), but it gives the player immediate
		// visual feedback, without which their game would feel laggy.
		if (!HasAuthority())
		{
			PlayFireEffects();

			// Run a cosmetic line trace just to see whether we should spawn an
			// impact effect
			FHitResult Hit;
			if (RunFireTrace(Hit))
			{
				const bool bWillProbablyCauseDamage = Hit.HitObjectHandle.IsValid() && Hit.HitObjectHandle.FetchActor()->CanBeDamaged();
				PlayImpactEffects(Hit.ImpactPoint, Hit.ImpactNormal, bWillProbablyCauseDamage);
			}
		}
	}
	else
	{
		// If the weapon is still on cooldown, play a click sound
		PlayUnableToFireEffects();
	}
}

void AWeapon::Server_TryFire_Implementation(const FVector& MuzzleLocation, const FVector& Direction)
{
	// We're now running with authority: whereas HandleFireInput is repsonsible
	// for responding to the input by deciding if we should ask the server to
	// fire, Server_TryFire is responsible for making the final, authoritative
	// decision about whether we should fire a shot. We need to do our own
	// cooldown check with authority; we can't simply trust the client, since
	// simple hacks would allow a player to bypass the cooldown check and issue
	// server RPCs whenever they wanted.
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float ElapsedSinceLastFire = CurrentTime - LastFireTime;
	if (ElapsedSinceLastFire >= FireCooldown)
	{
		// Cache our last fire time: note that LastFireTime isn't replicated;
		// it's updated independently on the server and on clients
		LastFireTime = CurrentTime;

		// Update our LastFirePacket to reflect that the server has allowed the
		// weapon to fire: this will replicate to non-owning clients (i.e. it
		// won't be sent to the player who originally issued this RPC), causing
		// their OnRep_LastFirePacket function to be called
		LastFirePacket.ServerFireTime = CurrentTime;

		// Run a server-authoritative line trace to determine if there's
		// any blocking geometry in the path of the shot, and if so, whether
		// that's a primitive belonging to an actor who we might deal damage to
		FHitResult Hit;
		if (RunFireTrace(Hit))
		{
			// If we hit a damageable actor, attempt to damage it
			float DamageCaused = 0.0f;
			if (Hit.HitObjectHandle.IsValid() && Hit.HitObjectHandle.FetchActor()->CanBeDamaged())
			{
				const float BaseDamage = 1.0f;
				const FPointDamageEvent DamageEvent(BaseDamage, Hit, Direction, UDamageType_WeaponFire::StaticClass());
				DamageCaused = Hit.HitObjectHandle.FetchActor()->TakeDamage(BaseDamage, DamageEvent, GetInstigatorController(), this);
			}

			// Spawn particle effects and audio server-side: note that we're
			// using UGameplayStatics functions which have no effect on
			// dedicated server, so while we could gate these calls behind
			// IsRunningDedicatedServer(), it's not strictly necessary. Either
			// way, these function calls take effect if we're running standalone
			// or as listen server.
			PlayFireEffects();
			PlayImpactEffects(Hit.ImpactPoint, Hit.ImpactNormal, DamageCaused > 0.0f);

			// Propagate the details of our hit to non-owning clients: this
			// gives them everything they need to know in order to spawn their
			// own cosmetic effectst
			LastFirePacket.bCausedDamage = DamageCaused > 0.0f;
			LastFirePacket.ImpactPoint = Hit.ImpactPoint;
			LastFirePacket.ImpactNormal = Hit.ImpactNormal;
		}
		else
		{
			// Set ImpactNormal to zero as a sentinel to indicate that this
			// shot didn't hit anything
			LastFirePacket.ImpactNormal = FVector::ZeroVector;
		}
	}
}

void AWeapon::OnRep_LastFirePacket()
{
	// LastFirePacket is replicated with COND_SkipOwner, so if we get this
	// notify, that means we're a remote client (i.e. not the client that owns
	// this weapon) -- this weapon belongs to a non-local player. Since all
	// the gameplay-authoritative stuff happens separately on the server, all we
	// need to do here is spawn cosmetic effects so that the local player can
	// see that this weapon has just been fired.
	PlayFireEffects();

	if (!LastFirePacket.ImpactNormal.IsZero())
	{
		PlayImpactEffects(LastFirePacket.ImpactPoint, LastFirePacket.ImpactNormal, LastFirePacket.bCausedDamage);
	}
}

void AWeapon::PlayFireEffects()
{
	// This is yet another instance where defining the particulars of our
	// cosmetic effects in Blueprints would make more sense: you'd typically
	// expose these events via a BlueprintCallable function. Also note that
	// we're spawning UParticleSystemComponents and UAudioComponents at runtime,
	// which carries some overhead, but it allows us to fire-and-forget our
	// effects without having to worry about overlap.
	if (FireEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(FireEffect, MuzzleHandle);
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, MuzzleHandle->GetComponentLocation(), MuzzleHandle->GetComponentRotation());
	}
}

void AWeapon::PlayUnableToFireEffects()
{
	if (DryFireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DryFireSound, MuzzleHandle->GetComponentLocation(), MuzzleHandle->GetComponentRotation());
	}
}

void AWeapon::PlayImpactEffects(const FVector& ImpactPoint, const FVector& ImpactNormal, bool bCausedDamage)
{
	const FRotator ImpactRotation = ImpactNormal.ToOrientationRotator();

	if (ImpactEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireEffect, ImpactPoint, ImpactRotation);
	}

	USoundBase* Sound = bCausedDamage ? DamagingImpactSound : NonDamagingImpactSound;
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, ImpactPoint, ImpactRotation);
	}
}

bool AWeapon::RunFireTrace(FHitResult& OutHit)
{
	const FVector& TraceStart = MuzzleHandle->GetComponentLocation();
	const FVector TraceEnd = TraceStart + (MuzzleHandle->GetForwardVector() * 5000.0f);
	const FName ProfileName = UCollisionProfile::BlockAllDynamic_ProfileName;
	const FCollisionQueryParams QueryParams(TEXT("WeaponFire"), false, GetOwner());
	return GetWorld()->LineTraceSingleByProfile(OutHit, TraceStart, TraceEnd, ProfileName, QueryParams);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, LastFirePacket, COND_SkipOwner);
}
