#pragma once

#include "CoreMinimal.h"
#include "ArsenalProjectileBase.h"
#include "VaccineDartProjectile.generated.h"

/**
 * Vaccine Pistol dart. On impact, applies a dose to the hit actor's UVaccinationComponent (if any)
 * and calls ApplyVaccine() on it if it implements IVaccineTarget. Ties into the CDC vaccine narrative.
 */
UCLASS()
class KNIGHTHACKS9JAM_API AVaccineDartProjectile : public AArsenalProjectileBase
{
	GENERATED_BODY()

public:
	AVaccineDartProjectile();

protected:
	virtual void OnProjectileImpact(const FHitResult& Hit) override;
};
