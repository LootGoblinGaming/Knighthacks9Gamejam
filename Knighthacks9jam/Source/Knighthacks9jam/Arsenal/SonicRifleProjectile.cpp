#include "SonicRifleProjectile.h"

ASonicRifleProjectile::ASonicRifleProjectile()
{
	// loud and far-reaching so it overrides the horde's current path vector
	NoiseLoudness = 100.0f;
	NoiseRange = 6000.0f;
	NoiseTag = FName("SonicLure");

	// no lingering effect - the noise event is instantaneous, destroy right after impact
	DeferredDestructionTime = 0.0f;
}
