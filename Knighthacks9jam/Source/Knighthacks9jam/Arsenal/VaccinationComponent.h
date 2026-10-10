#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VaccinationComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVaccinated, AActor*, VaccineInstigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVaccineDoseReceived, AActor*, VaccineInstigator, int32, DosesReceived, int32, DosesRequired);

/**
 * Add to any zombie Character to make it curable by the Vaccine Pistol. Once enough doses land,
 * the owner is marked vaccinated, its AI movement is stopped and its MaxWalkSpeed is locked to
 * CuredWalkSpeed, so it drops out of the horde even if its AI keeps issuing MoveTo requests.
 * Zombie logic (attacks, MoveTo loops) can check IsVaccinated() or bind OnVaccinated.
 */
UCLASS(ClassGroup = (Arsenal), meta = (BlueprintSpawnableComponent))
class KNIGHTHACKS9JAM_API UVaccinationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVaccinationComponent();

	/** Number of dart hits needed to cure the owner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vaccine", meta = (ClampMin = 1))
	int32 DosesRequired = 1;

	/** MaxWalkSpeed applied to the owner once cured. 0 makes the cured zombie stand still. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vaccine", meta = (ClampMin = 0))
	float CuredWalkSpeed = 0.0f;

	/** Fired once, when the final required dose lands */
	UPROPERTY(BlueprintAssignable, Category = "Vaccine")
	FOnVaccinated OnVaccinated;

	/** Fired for every dose that lands before the owner is cured, including the curing dose */
	UPROPERTY(BlueprintAssignable, Category = "Vaccine")
	FOnVaccineDoseReceived OnDoseReceived;

	/** Applies one vaccine dose. Returns true if this dose cured the owner. */
	UFUNCTION(BlueprintCallable, Category = "Vaccine")
	bool ApplyDose(AActor* VaccineInstigator);

	UFUNCTION(BlueprintPure, Category = "Vaccine")
	bool IsVaccinated() const { return bVaccinated; }

	UFUNCTION(BlueprintPure, Category = "Vaccine")
	int32 GetDosesReceived() const { return DosesReceived; }

private:
	int32 DosesReceived = 0;
	bool bVaccinated = false;
};
