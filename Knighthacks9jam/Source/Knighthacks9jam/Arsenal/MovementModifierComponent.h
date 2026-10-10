#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MovementModifierComponent.generated.h"

class ACharacter;

/**
 * Single owner of a Character's MaxWalkSpeed while arsenal effects are active. The owner reports its
 * normal speed through SetBaseSpeed() (e.g. on AI state changes) instead of writing MaxWalkSpeed directly,
 * and this component layers the Inhibitor Rifle slow, Bio-Foam freeze and vaccine lock on top, so no
 * effect can be undone by a state change or by another effect ending.
 */
UCLASS(ClassGroup = (Arsenal), meta = (BlueprintSpawnableComponent))
class KNIGHTHACKS9JAM_API UMovementModifierComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMovementModifierComponent();

	/** Accumulated slows can't push the walk speed below this (unless the base speed itself is lower, e.g. 0 while attacking) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Modifiers", meta = (ClampMin = 0))
	float MinSlowedSpeed = 100.0f;

	/** Sets the owner's unmodified walk speed. Call this wherever the owner would otherwise set MaxWalkSpeed. */
	UFUNCTION(BlueprintCallable, Category = "Movement Modifiers")
	void SetBaseSpeed(float NewBaseSpeed);

	/** Permanently reduces walk speed by a flat amount (stacks, floored at MinSlowedSpeed) */
	UFUNCTION(BlueprintCallable, Category = "Movement Modifiers")
	void AddSlow(float Amount);

	/** Sets walk speed to 0 for Duration seconds. Re-freezing extends the freeze if the new duration is longer. */
	UFUNCTION(BlueprintCallable, Category = "Movement Modifiers")
	void Freeze(float Duration);

	/** Permanently locks walk speed to LockedSpeed, overriding every other modifier (used by the vaccine cure) */
	UFUNCTION(BlueprintCallable, Category = "Movement Modifiers")
	void LockSpeed(float LockedSpeed);

	UFUNCTION(BlueprintPure, Category = "Movement Modifiers")
	float GetEffectiveSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Movement Modifiers")
	bool IsFrozen() const { return bFrozen; }

	/** Returns the Character's modifier component, adding one at runtime if it has none, so arsenal effects work on any Character */
	static UMovementModifierComponent* FindOrAdd(ACharacter* Character);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:
	/** Negative until set, so BeginPlay knows to capture the owner's current MaxWalkSpeed */
	float BaseSpeed = -1.0f;
	float TotalSlow = 0.0f;
	bool bFrozen = false;
	bool bSpeedLocked = false;
	float LockedSpeedValue = 0.0f;

	FTimerHandle FreezeTimer;

	void CaptureBaseSpeedIfUnset();
	void OnFreezeEnded();
	void ApplySpeed();
};
