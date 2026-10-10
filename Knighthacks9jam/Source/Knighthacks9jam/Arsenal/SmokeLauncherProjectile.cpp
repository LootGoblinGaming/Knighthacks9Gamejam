#include "SmokeLauncherProjectile.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

ASmokeLauncherProjectile::ASmokeLauncherProjectile()
{
	// smoke needs a few seconds to be useful as cover; keep the projectile alive that long after impact
	DeferredDestructionTime = 6.0f;
	NoiseTag = FName("SmokeImpact");

	VisionBlocker = CreateDefaultSubobject<USphereComponent>(TEXT("VisionBlocker"));
	VisionBlocker->SetupAttachment(RootComponent);
	VisionBlocker->SetSphereRadius(SmokeRadius);

	// only block the Visibility trace channel (what AIPerception sight uses); ignore physical collision entirely
	VisionBlocker->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VisionBlocker->SetCollisionResponseToAllChannels(ECR_Ignore);
	VisionBlocker->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	VisionBlocker->SetVisibility(false);
}

void ASmokeLauncherProjectile::OnProjectileImpact(const FHitResult& Hit)
{
	Super::OnProjectileImpact(Hit);

	VisionBlocker->SetSphereRadius(SmokeRadius);
	VisionBlocker->SetVisibility(true);

	if (SmokeEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SmokeEffect, GetActorLocation(), FRotator::ZeroRotator, FVector(SmokeRadius / 100.0f));
	}
}
