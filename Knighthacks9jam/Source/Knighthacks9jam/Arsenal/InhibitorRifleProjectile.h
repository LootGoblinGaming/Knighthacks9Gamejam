#pragma once

#include "CoreMinimal.h"
#include "ArsenalProjectileBase.h"
#include "InhibitorRifleProjectile.generated.h"

/**
 * Inhibitor Rifle round (hitscan). Each hit on a Character permanently shaves a flat amount off its
 * walk speed through its UMovementModifierComponent, floored at that component's MinSlowedSpeed.
 */
UCLASS()
class KNIGHTHACKS9JAM_API AInhibitorRifleProjectile : public AArsenalProjectileBase
{
	GENERATED_BODY()

public:
	AInhibitorRifleProjectile();

protected:
	/** Walk speed removed from the hit Character per hit */
	UPROPERTY(EditAnywhere, Category = "Inhibitor", meta = (ClampMin = 0))
	float SlowPerHit = 50.0f;

	virtual void OnProjectileImpact(const FHitResult& Hit) override;
};
