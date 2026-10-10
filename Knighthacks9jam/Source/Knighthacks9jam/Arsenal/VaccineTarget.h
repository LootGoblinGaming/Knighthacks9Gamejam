#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VaccineTarget.generated.h"

UINTERFACE(BlueprintType)
class KNIGHTHACKS9JAM_API UVaccineTarget : public UInterface
{
	GENERATED_BODY()
};

/**
 * Optional hook for actors that need custom vaccine reactions beyond UVaccinationComponent
 * (which handles the standard cure). Implementable in C++ or Blueprint.
 */
class KNIGHTHACKS9JAM_API IVaccineTarget
{
	GENERATED_BODY()

public:
	/** Called on the hit actor when a vaccine dart lands */
	UFUNCTION(BlueprintNativeEvent, Category = "Vaccine")
	void ApplyVaccine(AActor* Instigator);
};
