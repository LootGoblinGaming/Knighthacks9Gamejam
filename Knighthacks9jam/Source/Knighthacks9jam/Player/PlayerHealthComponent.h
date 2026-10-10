#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerHit, AActor*, Attacker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerRecovered);

/**
 * Two-hit health, CoD Zombies style. The first hit leaves the player Damaged; if they avoid being hit
 * again for RecoveryTime they're back to full. A hit while Damaged kills them.
 * Zombies hurt the player by calling TakeHit(), or by applying any engine damage (ApplyDamage) to the owner.
 */
UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class KNIGHTHACKS9JAM_API UPlayerHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerHealthComponent();

	/** Seconds without being hit before a Damaged player recovers. The damage overlay fades out over this time. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = 0.5, Units = "s"))
	float RecoveryTime = 5.0f;

	/** Grace period after a hit during which further hits are ignored, so one attack can't land twice */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = 0, Units = "s"))
	float HitInvulnerabilityTime = 1.0f;

	/** Fired on the first hit, when the player becomes Damaged */
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnPlayerHit OnDamaged;

	/** Fired when a Damaged player goes long enough without a hit to recover */
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnPlayerRecovered OnRecovered;

	/** Fired on the killing hit */
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnPlayerHit OnDied;

	/** Hits the player once. Returns true if the hit counted (not dead, not in the post-hit grace period). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	bool TakeHit(AActor* Attacker);

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDamaged() const { return bDamaged; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bDead; }

	/** 1 right after the first hit, fading to 0 as the player recovers; 1 once dead */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetDamageOverlayAlpha() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:
	bool bDamaged = false;
	bool bDead = false;
	double LastHitTime = -1.0e9;

	FTimerHandle RecoveryTimer;

	UFUNCTION()
	void OnOwnerTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	void Recover();
};
