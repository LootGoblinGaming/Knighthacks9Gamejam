#pragma once

#include "CoreMinimal.h"
#include "ArsenalProjectileBase.h"
#include "SonicRifleProjectile.generated.h"

/**
 * Sonic Rifle round. Rubber/sonic ammo with no damage payload - its only job is to
 * make a loud, distinctly-tagged noise on impact so AI hearing perception reroutes
 * the horde away from a chokepoint and toward the impact point.
 */
UCLASS()
class KNIGHTHACKS9JAM_API ASonicRifleProjectile : public AArsenalProjectileBase
{
	GENERATED_BODY()

public:
	ASonicRifleProjectile();
};
