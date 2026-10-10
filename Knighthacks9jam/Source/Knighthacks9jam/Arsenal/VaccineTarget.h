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
 * Implement on anything the Vaccine Pistol dart should be able to cure (zombie AI controllers
 * or characters). Lets the AI Architect's Behavior Tree react to a vaccine hit - whether
 * implemented in C++ or Blueprint - without the dart needing to know about the BT asset.
 */
class KNIGHTHACKS9JAM_API IVaccineTarget
{
	GENERATED_BODY()

public:
	/** Called on the hit actor when a vaccine dart lands. Implementer should switch its Behavior Tree to Neutral. */
	UFUNCTION(BlueprintNativeEvent, Category = "Vaccine")
	void ApplyVaccine(AActor* Instigator);
};
