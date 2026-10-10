#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArsenalWeaponBase.generated.h"

class AArsenalProjectileBase;
class UStaticMeshComponent;
class USceneComponent;
class USoundBase;
class UNiagaraSystem;
class UCameraShakeBase;

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

	/** Hitscan only: the round visibly flies from the muzzle to the hit point and lands the hit on arrival, instead of appearing there */
	UPROPERTY(EditAnywhere, Category = "Aim", meta = (EditCondition = "bHitscanWeapon"))
	bool bHitscanVisualFlight = true;

	/** Speed of the visible round on its way to the hit point */
	UPROPERTY(EditAnywhere, Category = "Aim", meta = (EditCondition = "bHitscanWeapon && bHitscanVisualFlight", ClampMin = 500, Units = "CentimetersPerSecond"))
	float HitscanVisualSpeed = 9000.0f;

	/** Long shots speed the round up so the hit never lands later than this after the trigger pull */
	UPROPERTY(EditAnywhere, Category = "Aim", meta = (EditCondition = "bHitscanWeapon && bHitscanVisualFlight", ClampMin = 0.01, ClampMax = 1, Units = "s"))
	float HitscanMaxFlightTime = 0.2f;

	/** Played at the muzzle on every shot */
	UPROPERTY(EditAnywhere, Category = "Feedback")
	TObjectPtr<USoundBase> FireSound;

	/** Spawned at the muzzle on every shot */
	UPROPERTY(EditAnywhere, Category = "Feedback")
	TObjectPtr<UNiagaraSystem> MuzzleFlashEffect;

	/** Upward camera kick per shot, for player owners */
	UPROPERTY(EditAnywhere, Category = "Feedback", meta = (ClampMin = 0, ClampMax = 10, Units = "Degrees"))
	float RecoilPitch = 0.6f;

	/** Max random sideways camera kick per shot, either direction */
	UPROPERTY(EditAnywhere, Category = "Feedback", meta = (ClampMin = 0, ClampMax = 10, Units = "Degrees"))
	float RecoilYawJitter = 0.2f;

	/** Optional camera shake played on the owning player per shot */
	UPROPERTY(EditAnywhere, Category = "Feedback")
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	bool bIsFiring = false;
	FTimerHandle RefireTimer;

	/** World time of the last shot, so RefireRate also limits how fast repeated clicks can fire */
	double LastFireTime = -1.0e9;

	/** Cast pawn pointer to the owner, used for hitscan aim tracing and as the projectile Instigator */
	UPROPERTY()
	TObjectPtr<APawn> OwningPawn;

	virtual void BeginPlay() override;

	/** Returns the direction projectiles should be fired in. Override once this is wired up to a player camera. */
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	FVector GetAimDirection() const;
	virtual FVector GetAimDirection_Implementation() const;

	virtual void Fire();

	/** Sound, muzzle flash, recoil and camera shake for one shot */
	void PlayFireFeedback();

	/** Traces from the owning Pawn's eyes along its look direction and applies the impact at wherever the trace lands */
	virtual void FireHitscan();

public:
	UFUNCTION(BlueprintPure, Category = "Weapon")
	UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
};
