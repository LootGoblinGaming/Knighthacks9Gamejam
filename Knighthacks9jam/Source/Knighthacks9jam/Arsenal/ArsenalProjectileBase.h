#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArsenalProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class USoundBase;
class UArrowComponent;
class UNiagaraSystem;

/**
 * Base projectile for the non-lethal arsenal (Bio-Foam Payload, Inhibitor Rifle, Vaccine Pistol).
 * Handles movement, collision, and the single-hit lifecycle. Subclasses override
 * OnProjectileImpact() for weapon-specific behavior.
 */
UCLASS(Abstract)
class KNIGHTHACKS9JAM_API AArsenalProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AArsenalProjectileBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** Editor-only: points the way the projectile flies. Rotate added meshes so their tip lines up with it. */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UArrowComponent> FlightDirectionArrow;

protected:
	/** Loudness of the AI perception noise made by this projectile on impact */
	UPROPERTY(EditAnywhere, Category = "Projectile|Noise", meta = (ClampMin = 0, ClampMax = 100))
	float NoiseLoudness = 3.0f;

	/** Range of the AI perception noise made by this projectile on impact */
	UPROPERTY(EditAnywhere, Category = "Projectile|Noise", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float NoiseRange = 3000.0f;

	/** Tag of the AI perception noise made by this projectile on impact */
	UPROPERTY(EditAnywhere, Category = "Projectile|Noise")
	FName NoiseTag = FName("Projectile");

	/** How long to wait after impact before destroying this projectile */
	UPROPERTY(EditAnywhere, Category = "Projectile|Destruction", meta = (ClampMin = 0, ClampMax = 30, Units = "s"))
	float DeferredDestructionTime = 2.0f;

	/** Played at the impact point */
	UPROPERTY(EditAnywhere, Category = "Projectile|Feedback")
	TObjectPtr<USoundBase> ImpactSound;

	/** Spawned at the impact point, facing out of the surface */
	UPROPERTY(EditAnywhere, Category = "Projectile|Feedback")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	/** Purely visual: on impact, bounce off the surface spinning (and tumble along the ground) until destroyed, instead of stopping dead */
	UPROPERTY(EditAnywhere, Category = "Projectile|Ricochet")
	bool bRicochetOnImpact = false;

	/** Speed the projectile leaves the surface at */
	UPROPERTY(EditAnywhere, Category = "Projectile|Ricochet", meta = (EditCondition = "bRicochetOnImpact", ClampMin = 0, Units = "CentimetersPerSecond"))
	float RicochetSpeed = 700.0f;

	/** Random spread applied to the reflected direction, so repeated ricochets don't all look identical */
	UPROPERTY(EditAnywhere, Category = "Projectile|Ricochet", meta = (EditCondition = "bRicochetOnImpact", ClampMin = 0, ClampMax = 90, Units = "Degrees"))
	float RicochetSpreadAngle = 25.0f;

	/** Tumble rate while ricocheting; each axis gets a random direction */
	UPROPERTY(EditAnywhere, Category = "Projectile|Ricochet", meta = (EditCondition = "bRicochetOnImpact"))
	FRotator RicochetSpinRate = FRotator(900.0f, 600.0f, 0.0f);

	/** True once this projectile has impacted something, guards against double-processing a hit */
	bool bHit = false;

	FTimerHandle DestructionTimer;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	/** Weapon-specific impact behavior. Called once per projectile, after noise has been made. */
	virtual void OnProjectileImpact(const FHitResult& Hit);

	/** Blueprint hook for VFX/SFX on impact, fired after OnProjectileImpact() */
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile", meta = (DisplayName = "On Projectile Impact"))
	void BP_OnProjectileImpact(const FHitResult& Hit);

public:
	/**
	 * Applies this projectile's impact behavior immediately, without requiring an actual physics collision.
	 * Used by hitscan weapons: they spawn this projectile at the traced hit location and call this directly
	 * so the noise / interface / Blueprint impact hooks all fire identically to a physically-travelled hit.
	 */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SimulateInstantHit(const FHitResult& Hit);

	/**
	 * Hitscan presentation: flies (visual only, no collision) from where it was spawned to Hit.ImpactPoint at Speed,
	 * sped up if needed so the trip never takes longer than MaxFlightTime, then applies Hit on arrival.
	 */
	void LaunchHitscanFlight(const FHitResult& Hit, float Speed, float MaxFlightTime);

private:
	FRotator ActiveSpinRate = FRotator::ZeroRotator;

	FHitResult PendingHitscanHit;
	FTimerHandle HitscanFlightTimer;

	void OnHitscanFlightArrived();

	void ProcessImpact(const FHitResult& Hit);
	void StartRicochet(const FHitResult& Hit);

	UFUNCTION()
	void OnRicochetStopped(const FHitResult& ImpactResult);
	void OnDeferredDestruction();
};
