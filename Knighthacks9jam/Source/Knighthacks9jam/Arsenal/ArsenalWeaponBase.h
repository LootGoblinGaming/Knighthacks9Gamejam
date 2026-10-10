#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArsenalWeaponBase.generated.h"

class AArsenalProjectileBase;
class UStaticMeshComponent;
class USceneComponent;

/**
 * Base class for the non-lethal arsenal weapons (Bio-Foam Payload, Inhibitor Rifle, Vaccine Pistol).
 * Self-contained so it can be dropped into a blank test map and fired via Fire()
 * before the first-person character integration lands.
 */
UCLASS(Abstract)
class KNIGHTHACKS9JAM_API AArsenalWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AArsenalWeaponBase();

	/** Starts firing. For full-auto weapons this keeps firing at RefireRate until StopFiring() is called. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFiring();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFiring();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	/** Where projectiles spawn from and the direction they're aimed, in the absence of a camera-based aim override */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> MuzzlePoint;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	TSubclassOf<AArsenalProjectileBase> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Refire")
	bool bFullAuto = false;

	UPROPERTY(EditAnywhere, Category = "Refire", meta = (ClampMin = 0.05, ClampMax = 5, Units = "s"))
	float RefireRate = 0.5f;

	/** If true, this weapon fires an instant trace from the owning Pawn's eyes instead of spawning a physically-travelling projectile. */
	UPROPERTY(EditAnywhere, Category = "Aim")
	bool bHitscanWeapon = false;

	/** Max trace distance for hitscan weapons */
	UPROPERTY(EditAnywhere, Category = "Aim", meta = (EditCondition = "bHitscanWeapon", ClampMin = 100, ClampMax = 50000, Units = "cm"))
	float HitscanRange = 10000.0f;

	bool bIsFiring = false;
	FTimerHandle RefireTimer;

	/** Cast pawn pointer to the owner, used for hitscan aim tracing and as the projectile Instigator */
	UPROPERTY()
	TObjectPtr<APawn> OwningPawn;

	virtual void BeginPlay() override;

	/** Returns the direction projectiles should be fired in. Override once this is wired up to a player camera. */
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	FVector GetAimDirection() const;
	virtual FVector GetAimDirection_Implementation() const;

	virtual void Fire();

	/** Traces from the owning Pawn's eyes along its look direction and applies the impact at wherever the trace lands */
	virtual void FireHitscan();

public:
	UFUNCTION(BlueprintPure, Category = "Weapon")
	UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
};
