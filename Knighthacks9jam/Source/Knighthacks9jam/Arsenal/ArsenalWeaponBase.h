#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArsenalWeaponBase.generated.h"

class AArsenalProjectileBase;
class UStaticMeshComponent;
class USceneComponent;

/**
 * Base class for the non-lethal arsenal weapons (Smoke Launcher, Sonic Rifle, Vaccine Pistol).
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

	bool bIsFiring = false;
	FTimerHandle RefireTimer;

	/** Returns the direction projectiles should be fired in. Override once this is wired up to a player camera. */
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	FVector GetAimDirection() const;
	virtual FVector GetAimDirection_Implementation() const;

	virtual void Fire();

public:
	UFUNCTION(BlueprintPure, Category = "Weapon")
	UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
};
