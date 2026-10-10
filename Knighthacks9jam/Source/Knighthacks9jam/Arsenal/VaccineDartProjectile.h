#pragma once

#include "CoreMinimal.h"
#include "ArsenalProjectileBase.h"
#include "VaccineDartProjectile.generated.h"

class USoundBase;
class UNiagaraSystem;
class UMaterialInterface;

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
	/** Played on the target when this dart's dose cures it */
	UPROPERTY(EditAnywhere, Category = "Vaccine|Feedback")
	TObjectPtr<USoundBase> CureSound;

	/** Spawned on the target when this dart's dose cures it */
	UPROPERTY(EditAnywhere, Category = "Vaccine|Feedback")
	TObjectPtr<UNiagaraSystem> CureEffect;

	/** Overlay material applied to every mesh on a cured target, so cured zombies are recognisable at a glance */
	UPROPERTY(EditAnywhere, Category = "Vaccine|Feedback")
	TObjectPtr<UMaterialInterface> CuredOverlayMaterial;

	/** Played when a dose lands but the target needs more to be cured */
	UPROPERTY(EditAnywhere, Category = "Vaccine|Feedback")
	TObjectPtr<USoundBase> DoseSound;

	/** Spawned when a dose lands but the target needs more to be cured */
	UPROPERTY(EditAnywhere, Category = "Vaccine|Feedback")
	TObjectPtr<UNiagaraSystem> DoseEffect;

	virtual void OnProjectileImpact(const FHitResult& Hit) override;

private:
	void PlayCureFeedback(AActor* CuredActor);
};
