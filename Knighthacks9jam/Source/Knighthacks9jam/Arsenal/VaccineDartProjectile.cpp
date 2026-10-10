#include "VaccineDartProjectile.h"
#include "VaccineTarget.h"

AVaccineDartProjectile::AVaccineDartProjectile()
{
	// quiet - this is a precision tool, not something meant to draw the horde
	NoiseLoudness = 1.0f;
	NoiseRange = 500.0f;
	NoiseTag = FName("VaccineDart");
}

void AVaccineDartProjectile::OnProjectileImpact(const FHitResult& Hit)
{
	Super::OnProjectileImpact(Hit);

	AActor* HitActor = Hit.GetActor();
	if (HitActor && HitActor->Implements<UVaccineTarget>())
	{
		IVaccineTarget::Execute_ApplyVaccine(HitActor, GetInstigator());
	}
}
