#include "InhibitorRifleProjectile.h"
#include "MovementModifierComponent.h"
#include "GameFramework/Character.h"

AInhibitorRifleProjectile::AInhibitorRifleProjectile()
{
	NoiseTag = FName("InhibitorImpact");

	// linger briefly so the round's mesh is visible at the impact point as hit feedback
	DeferredDestructionTime = 0.5f;
}

void AInhibitorRifleProjectile::OnProjectileImpact(const FHitResult& Hit)
{
	Super::OnProjectileImpact(Hit);

	if (UMovementModifierComponent* Modifier = UMovementModifierComponent::FindOrAdd(Cast<ACharacter>(Hit.GetActor())))
	{
		Modifier->AddSlow(SlowPerHit);
	}
}
