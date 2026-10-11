#include "PulsingPointLightComponent.h"

UPulsingPointLightComponent::UPulsingPointLightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIntensityUnits(ELightUnits::Candelas);
	Intensity = 15.0f;
	AttenuationRadius = 300.0f;
	LightColor = FLinearColor(0.25f, 1.0f, 0.3f).ToFColor(true);
	CastShadows = false;
}

void UPulsingPointLightComponent::BeginPlay()
{
	Super::BeginPlay();

	BaseIntensity = Intensity;

	// desync neighbouring pickups so they don't all breathe in lockstep
	RunningTime = FMath::FRandRange(0.0f, PulsePeriod);
}

void UPulsingPointLightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RunningTime += DeltaTime;
	const float Dip = 0.5f * (1.0f - FMath::Cos(RunningTime * UE_TWO_PI / PulsePeriod));
	SetIntensity(BaseIntensity * (1.0f - PulseAmount * Dip));
}
