#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArsenalWeaponPickup.generated.h"

class AArsenalWeaponBase;
class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class USoundBase;
class UNiagaraSystem;

/**
 * Walk-over weapon pickup. Displays WeaponClass's gun mesh spinning and bobbing in place; when a Pawn
 * with a UArsenalInventoryComponent walks into it, adds and equips that weapon, then disappears.
 * Pawns that already own the weapon (or have no inventory, like zombies) leave it where it is.
 */
UCLASS()
class KNIGHTHACKS9JAM_API AArsenalWeaponPickup : public AActor
{
	GENERATED_BODY()

public:
	AArsenalWeaponPickup();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> PickupTrigger;

	/** Shows the weapon's own mesh, copied from WeaponClass */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DisplayMesh;

	/** Soft pulsing glow so the pickup reads from a distance in a dark level; set colour/brightness on this component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> GlowLight;

protected:
	/** The weapon this pickup gives */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	TSubclassOf<AArsenalWeaponBase> WeaponClass;

	UPROPERTY(EditAnywhere, Category = "Pickup|Display", meta = (Units = "DegreesPerSecond"))
	float SpinSpeed = 90.0f;

	UPROPERTY(EditAnywhere, Category = "Pickup|Display", meta = (ClampMin = 0, Units = "cm"))
	float BobHeight = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Pickup|Display", meta = (ClampMin = 0, Units = "s"))
	float BobPeriod = 2.0f;

	/** How much the glow brightness breathes, as a fraction of its set intensity (0 = steady) */
	UPROPERTY(EditAnywhere, Category = "Pickup|Display", meta = (ClampMin = 0, ClampMax = 1))
	float GlowPulseAmount = 0.35f;

	/** Played at the pickup when it's collected */
	UPROPERTY(EditAnywhere, Category = "Pickup|Feedback")
	TObjectPtr<USoundBase> PickupSound;

	/** Spawned at the pickup when it's collected */
	UPROPERTY(EditAnywhere, Category = "Pickup|Feedback")
	TObjectPtr<UNiagaraSystem> PickupEffect;

	/** Length of the collect "pop": the gun swells then shrinks away while the glow fades */
	UPROPERTY(EditAnywhere, Category = "Pickup|Feedback", meta = (ClampMin = 0, ClampMax = 2, Units = "s"))
	float PopDuration = 0.25f;

	/** Blueprint hook for pickup sound/VFX, fired just before the pickup is destroyed */
	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup", meta = (DisplayName = "On Picked Up"))
	void BP_OnPickedUp(AActor* PickedUpBy);

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnPickupTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	FVector DisplayBaseLocation = FVector::ZeroVector;
	FVector DisplayBaseScale = FVector::OneVector;
	float GlowBaseIntensity = 0.0f;
	float RunningTime = 0.0f;

	bool bCollected = false;
	float PopElapsed = 0.0f;

	void CopyWeaponMesh();
};
