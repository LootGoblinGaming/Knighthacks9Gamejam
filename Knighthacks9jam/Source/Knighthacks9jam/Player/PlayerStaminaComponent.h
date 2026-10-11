#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "PlayerStaminaComponent.generated.h"

class UInputComponent;
class APlayerController;

/**
 * Hold-to-sprint with a stamina pool, BotW style. Sprinting drains stamina; it refills after a short pause.
 * Running it completely dry leaves the player Exhausted - no sprinting until it refills to RecoverFraction.
 * Add to the player Character; binds its own sprint key, so no Blueprint input changes are needed.
 */
UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class KNIGHTHACKS9JAM_API UPlayerStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerStaminaComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = 1))
	float MaxStamina = 100.0f;

	/** Stamina used per second while sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = 0))
	float DrainPerSecond = 20.0f;

	/** Stamina regained per second once regeneration starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = 0))
	float RegenPerSecond = 25.0f;

	/** Pause after sprinting stops before stamina starts refilling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = 0, Units = "s"))
	float RegenDelay = 1.0f;

	/** When Exhausted, sprinting unlocks again once stamina refills to this fraction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = 0, ClampMax = 1))
	float RecoverFraction = 0.4f;

	/** Walk speed is multiplied by this while sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint", meta = (ClampMin = 1))
	float SprintSpeedMultiplier = 1.7f;

	UPROPERTY(EditAnywhere, Category = "Sprint")
	FKey SprintKey = EKeys::LeftShift;

	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetStaminaFraction() const { return MaxStamina > 0.0f ? Stamina / MaxStamina : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool IsSprinting() const { return bSprinting; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool IsExhausted() const { return bExhausted; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float Stamina = 0.0f;
	float WalkSpeed = 0.0f;
	float TimeSinceSprint = 0.0f;
	bool bSprintHeld = false;
	bool bSprinting = false;
	bool bExhausted = false;

	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY()
	TObjectPtr<UInputComponent> SprintInput;

	UFUNCTION()
	void OnOwnerControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	void SetupForController(APlayerController* NewPlayerController);
	void OnSprintPressed() { bSprintHeld = true; }
	void OnSprintReleased() { bSprintHeld = false; }
	void SetSprinting(bool bNewSprinting);
};
