#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArsenalProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

/**
 * Base projectile for the non-lethal arsenal (Smoke Launcher, Sonic Rifle, Vaccine Pistol).
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

	/** True once this projectile has impacted something, guards against double-processing a hit */
	bool bHit = false;

	FTimerHandle DestructionTimer;

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	/** Weapon-specific impact behavior. Called once per projectile, after noise has been made. */
	virtual void OnProjectileImpact(const FHitResult& Hit);

	/** Blueprint hook for VFX/SFX on impact, fired after OnProjectileImpact() */
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile", meta = (DisplayName = "On Projectile Impact"))
	void BP_OnProjectileImpact(const FHitResult& Hit);

private:
	void OnDeferredDestruction();
};
