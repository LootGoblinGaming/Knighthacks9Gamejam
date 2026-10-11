#pragma once

#include "CoreMinimal.h"
#include "Components/PointLightComponent.h"
#include "PulsingPointLightComponent.generated.h"

/**
 * Point light that gently breathes in brightness - add it to a pickup Blueprint to make the pickup glow
 * and read from a distance in a dark level. Defaults to a soft green, shadowless glow.
 */
UCLASS(ClassGroup = (Lights), meta = (BlueprintSpawnableComponent))
class KNIGHTHACKS9JAM_API UPulsingPointLightComponent : public UPointLightComponent
{
	GENERATED_BODY()

public:
	UPulsingPointLightComponent();

	/** How much the brightness dips each pulse, as a fraction of Intensity (0 = steady glow) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pulse", meta = (ClampMin = 0, ClampMax = 1))
	float PulseAmount = 0.35f;

	/** Seconds per full pulse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pulse", meta = (ClampMin = 0.1, Units = "s"))
	float PulsePeriod = 2.0f;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float BaseIntensity = 0.0f;
	float RunningTime = 0.0f;
};
