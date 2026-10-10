#include "BioFoamProjectile.h"
#include "MovementModifierComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

ABioFoamProjectile::ABioFoamProjectile()
{
	NoiseTag = FName("BioFoamImpact");

	FoamZone = CreateDefaultSubobject<USphereComponent>(TEXT("FoamZone"));
	FoamZone->SetupAttachment(RootComponent);
	FoamZone->SetSphereRadius(FoamRadius);

	// overlap Pawns only, never block anything; stays off until the grenade lands
	FoamZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FoamZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	FoamZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	FoamZone->SetGenerateOverlapEvents(true);

	FoamVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FoamVisual"));
	FoamVisual->SetupAttachment(RootComponent);
	FoamVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FoamVisual->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		FoamVisual->SetStaticMesh(SphereMesh.Object);
	}
}

void ABioFoamProjectile::BeginPlay()
{
	Super::BeginPlay();

	FoamZone->OnComponentBeginOverlap.AddDynamic(this, &ABioFoamProjectile::OnFoamZoneBeginOverlap);
}

void ABioFoamProjectile::OnProjectileImpact(const FHitResult& Hit)
{
	Super::OnProjectileImpact(Hit);

	// keep the projectile (and its foam zone) alive for as long as the foam lasts
	DeferredDestructionTime = FoamLifetime;

	FoamZone->SetSphereRadius(FoamRadius);
	FoamZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FoamZone->UpdateOverlaps();

	// catch everything already standing in the blast, in case enabling collision didn't raise begin-overlap for them
	TArray<AActor*> OverlappingActors;
	FoamZone->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());
	for (AActor* Actor : OverlappingActors)
	{
		FreezeActor(Actor);
	}

	// engine sphere mesh is 100cm across; flatten it into a dome
	const float VisualScale = FoamRadius / 50.0f;
	FoamVisual->SetWorldRotation(FRotator::ZeroRotator);
	FoamVisual->SetWorldScale3D(FVector(VisualScale, VisualScale, VisualScale * 0.35f));
	FoamVisual->SetVisibility(true);

	if (FoamEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FoamEffect, GetActorLocation(), FRotator::ZeroRotator, FVector(FoamRadius / 100.0f));
	}
}

void ABioFoamProjectile::OnFoamZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FreezeActor(OtherActor);
}

void ABioFoamProjectile::FreezeActor(AActor* Actor)
{
	// the thrower is immune to their own foam
	if (!Actor || Actor == GetInstigator())
	{
		return;
	}

	if (UMovementModifierComponent* Modifier = UMovementModifierComponent::FindOrAdd(Cast<ACharacter>(Actor)))
	{
		Modifier->Freeze(FreezeDuration);
	}
}
