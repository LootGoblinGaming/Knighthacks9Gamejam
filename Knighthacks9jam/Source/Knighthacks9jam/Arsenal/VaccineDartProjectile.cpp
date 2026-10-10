#include "VaccineDartProjectile.h"
#include "VaccineTarget.h"
#include "VaccinationComponent.h"

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
	if (!HitActor)
	{
		return;
	}

	if (UVaccinationComponent* Vaccination = HitActor->FindComponentByClass<UVaccinationComponent>())
	{
		Vaccination->ApplyDose(GetInstigator());
	}

	if (HitActor->Implements<UVaccineTarget>())
	{
		IVaccineTarget::Execute_ApplyVaccine(HitActor, GetInstigator());
	}
}
