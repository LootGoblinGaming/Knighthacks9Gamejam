#include "ArsenalProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Components/MeshComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "ArsenalEffects.h"

AArsenalProjectileBase::AArsenalProjectileBase()
{
	// tick only runs while ricocheting, to spin the projectile
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	RootComponent = CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetSphereRadius(10.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	FlightDirectionArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("FlightDirectionArrow"));
	if (FlightDirectionArrow)
	{
		FlightDirectionArrow->SetupAttachment(RootComponent);
		FlightDirectionArrow->ArrowSize = 0.5f;
		FlightDirectionArrow->SetArrowColor(FLinearColor(1.0f, 0.4f, 0.0f));
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 2500.0f;
	ProjectileMovement->MaxSpeed = 2500.0f;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.5f;
}

void AArsenalProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	// don't let the projectile collide with the pawn that fired it
	CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);

	// meshes are visual only - CollisionComponent handles all hits. Designer-added meshes default to
	// BlockAllDynamic, which would block hitscan traces (e.g. lingering rounds eating the next shot) and pawns.
	TArray<UMeshComponent*> Meshes;
	GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* Mesh : Meshes)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AArsenalProjectileBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AddActorLocalRotation(ActiveSpinRate * DeltaSeconds);
}

void AArsenalProjectileBase::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(DestructionTimer);
	GetWorld()->GetTimerManager().ClearTimer(HitscanFlightTimer);
}

void AArsenalProjectileBase::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	ProcessImpact(Hit);
}

void AArsenalProjectileBase::SimulateInstantHit(const FHitResult& Hit)
{
	ProcessImpact(Hit);
}

void AArsenalProjectileBase::LaunchHitscanFlight(const FHitResult& Hit, float Speed, float MaxFlightTime)
{
	const FVector ToTarget = Hit.ImpactPoint - GetActorLocation();
	const float Distance = ToTarget.Size();
	if (Distance < 1.0f || Speed <= 0.0f)
	{
		ProcessImpact(Hit);
		return;
	}

	const float FlightSpeed = FMath::Max(Speed, Distance / FMath::Max(MaxFlightTime, 0.01f));
	PendingHitscanHit = Hit;

	// the hit was already decided by the trace; the flight is just for show, so it mustn't collide with anything on the way
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->MaxSpeed = 0.0f;
	ProjectileMovement->Velocity = ToTarget / Distance * FlightSpeed;
	SetActorRotation(ToTarget.Rotation());

	GetWorld()->GetTimerManager().SetTimer(HitscanFlightTimer, this, &AArsenalProjectileBase::OnHitscanFlightArrived, Distance / FlightSpeed, false);
}

void AArsenalProjectileBase::OnHitscanFlightArrived()
{
	SetActorLocation(PendingHitscanHit.ImpactPoint);
	ProcessImpact(PendingHitscanHit);
}

void AArsenalProjectileBase::ProcessImpact(const FHitResult& Hit)
{
	if (bHit)
	{
		return;
	}
	bHit = true;

	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovement->StopMovementImmediately();
	// without this, gravity keeps pulling the now collision-less projectile down through the floor
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	MakeNoise(NoiseLoudness, GetInstigator(), GetActorLocation(), NoiseRange, NoiseTag);

	// a hitscan miss has no real surface to play feedback on
	if (Hit.bBlockingHit)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Hit.ImpactPoint);
		ArsenalEffects::SpawnEffectAtLocation(this, ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}

	OnProjectileImpact(Hit);
	BP_OnProjectileImpact(Hit);

	// a hitscan miss has nothing to bounce off
	if (bRicochetOnImpact && Hit.bBlockingHit)
	{
		StartRicochet(Hit);
	}

	if (DeferredDestructionTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(DestructionTimer, this, &AArsenalProjectileBase::OnDeferredDestruction, DeferredDestructionTime, false);
	}
	else
	{
		Destroy();
	}
}

void AArsenalProjectileBase::OnProjectileImpact(const FHitResult& Hit)
{
	// base class has no weapon-specific behavior; subclasses override this
}

void AArsenalProjectileBase::StartRicochet(const FHitResult& Hit)
{
	FVector IncomingDirection = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal();
	if (IncomingDirection.IsNearlyZero())
	{
		IncomingDirection = -Hit.ImpactNormal;
	}

	FVector BounceDirection = FMath::GetReflectionVector(IncomingDirection, Hit.ImpactNormal);
	BounceDirection = FMath::VRandCone(BounceDirection, FMath::DegreesToRadians(RicochetSpreadAngle));

	// never bounce back into the surface
	if (FVector::DotProduct(BounceDirection, Hit.ImpactNormal) < 0.1f)
	{
		BounceDirection = (BounceDirection + Hit.ImpactNormal).GetSafeNormal();
	}

	// start just clear of the surface so the first sweep isn't already penetrating it
	SetActorLocation(Hit.ImpactPoint + Hit.ImpactNormal * (CollisionComponent->GetScaledSphereRadius() + 2.0f));

	// visual only: bounce off level geometry, never Pawns, so it can't re-hit or shove anyone
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.4f;
	ProjectileMovement->Friction = 0.3f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->MaxSpeed = 0.0f;
	ProjectileMovement->SetUpdatedComponent(CollisionComponent);
	ProjectileMovement->Velocity = BounceDirection * RicochetSpeed;
	ProjectileMovement->OnProjectileStop.AddUniqueDynamic(this, &AArsenalProjectileBase::OnRicochetStopped);

	ActiveSpinRate = FRotator(
		RicochetSpinRate.Pitch * (FMath::RandBool() ? 1.0f : -1.0f),
		RicochetSpinRate.Yaw * (FMath::RandBool() ? 1.0f : -1.0f),
		RicochetSpinRate.Roll * (FMath::RandBool() ? 1.0f : -1.0f));
	SetActorTickEnabled(true);
}

void AArsenalProjectileBase::OnRicochetStopped(const FHitResult& ImpactResult)
{
	// came to rest on the ground; stop tumbling
	SetActorTickEnabled(false);
}

void AArsenalProjectileBase::OnDeferredDestruction()
{
	Destroy();
}
