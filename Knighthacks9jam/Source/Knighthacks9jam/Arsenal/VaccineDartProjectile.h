#pragma once

#include "CoreMinimal.h"
#include "ArsenalProjectileBase.h"
#include "VaccineDartProjectile.generated.h"

/**
 * Vaccine Pistol dart. On impact with a Pawn implementing IVaccineTarget, calls ApplyVaccine()
 * so the zombie's Behavior Tree can switch to its Neutral state. Ties into the CDC vaccine narrative.
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
