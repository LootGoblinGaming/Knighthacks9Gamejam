#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "PlayerScreensComponent.generated.h"

class UUserWidget;
class UTexture2D;
class UInputComponent;
class APlayerController;
class UPlayerHealthComponent;
class USoundBase;
class UCameraShakeBase;
class UCameraComponent;

/**
 * Add to the player Character, next to UPlayerHealthComponent. Owns the player's full-screen states:
 * the damage overlay (driven by the health component), the pause menu (PauseKeys), and the game over
 * screen (on death). Works with no Blueprint changes; swap the widget classes for designed ones later.
 */
UCLASS(ClassGroup = (Player), meta = (BlueprintSpawnableComponent))
class KNIGHTHACKS9JAM_API UPlayerScreensComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerScreensComponent();

	UPROPERTY(EditAnywhere, Category = "Screens")
	TSubclassOf<UUserWidget> DamageOverlayClass;

	UPROPERTY(EditAnywhere, Category = "Screens")
	TSubclassOf<UUserWidget> PauseMenuClass;

	UPROPERTY(EditAnywhere, Category = "Screens")
	TSubclassOf<UUserWidget> GameOverClass;

	UPROPERTY(EditAnywhere, Category = "Screens")
	TSubclassOf<UUserWidget> VictoryClass;

	UPROPERTY(EditAnywhere, Category = "Screens")
	TSubclassOf<UUserWidget> MessageClass;

	/** Shown at the screen centre while the player is holding a weapon. Clear it to have no crosshair. */
	UPROPERTY(EditAnywhere, Category = "Screens")
	TSubclassOf<UUserWidget> CrosshairClass;

	/** Stamina ring around the crosshair, driven by a UPlayerStaminaComponent on the same actor. Clear it to hide. */
	UPROPERTY(EditAnywhere, Category = "Screens")
	TSubclassOf<UUserWidget> StaminaWheelClass;

	/** Optional always-on gameplay HUD (e.g. the supplies checklist), shown in every level this player is in */
	UPROPERTY(EditAnywhere, Category = "Screens")
	TSubclassOf<UUserWidget> HUDClass;

	/** Optional blood splatter image (with transparency) for the damage overlay; a red wash is used if empty */
	UPROPERTY(EditAnywhere, Category = "Screens")
	TObjectPtr<UTexture2D> BloodOverlayTexture;

	/** Keys that open/close the pause menu. Note: Escape also stops Play-In-Editor, so test with P in the editor. */
	UPROPERTY(EditAnywhere, Category = "Screens")
	TArray<FKey> PauseKeys;

	/** Level opened by the Main Menu buttons */
	UPROPERTY(EditAnywhere, Category = "Screens")
	FName MainMenuLevel = FName("MainMenu");

	/** Length of the death camera fall/fade between the killing hit and the game over screen */
	UPROPERTY(EditAnywhere, Category = "Screens", meta = (ClampMin = 0, Units = "s"))
	float GameOverDelay = 1.5f;

	/** Played (2D) when the player takes their first hit */
	UPROPERTY(EditAnywhere, Category = "Feedback|Hit")
	TObjectPtr<USoundBase> HitSound;

	/** On a hit the view snaps this far in a random direction, then eases back to where the player was aiming */
	UPROPERTY(EditAnywhere, Category = "Feedback|Hit", meta = (ClampMin = 0, ClampMax = 45, Units = "Degrees"))
	float HitViewKick = 8.0f;

	/** How long the view takes to ease back after a hit snap */
	UPROPERTY(EditAnywhere, Category = "Feedback|Hit", meta = (ClampMin = 0.05, ClampMax = 3, Units = "s"))
	float HitKickRecoverTime = 0.4f;

	/** Optional camera shake asset played on a hit, on top of the jolt */
	UPROPERTY(EditAnywhere, Category = "Feedback|Hit")
	TSubclassOf<UCameraShakeBase> HitCameraShake;

	/** Played (2D) on the killing hit */
	UPROPERTY(EditAnywhere, Category = "Feedback|Death")
	TObjectPtr<USoundBase> DeathSound;

	/** How far the view tips sideways as the player falls */
	UPROPERTY(EditAnywhere, Category = "Feedback|Death", meta = (ClampMin = -90, ClampMax = 90, Units = "Degrees"))
	float DeathTiltRoll = 35.0f;

	/** How far the view drops toward the ground as the player falls */
	UPROPERTY(EditAnywhere, Category = "Feedback|Death", meta = (ClampMin = 0, ClampMax = 80, Units = "Degrees"))
	float DeathPitchDrop = 40.0f;

	/** How far the camera sinks as the player falls */
	UPROPERTY(EditAnywhere, Category = "Feedback|Death", meta = (ClampMin = 0, Units = "cm"))
	float DeathCameraDrop = 120.0f;

	/** Screen fades to this colour over GameOverDelay */
	UPROPERTY(EditAnywhere, Category = "Feedback|Death")
	FLinearColor DeathFadeColor = FLinearColor::Black;

	/** Development only: hurts the player once, for testing without zombies. Ignored in shipping builds. */
	UPROPERTY(EditAnywhere, Category = "Screens|Debug")
	FKey DebugHurtKey = EKeys::K;

	UFUNCTION(BlueprintCallable, Category = "Screens")
	void TogglePause();

	UFUNCTION(BlueprintCallable, Category = "Screens")
	void PauseGame();

	UFUNCTION(BlueprintCallable, Category = "Screens")
	void ResumeGame();

	UFUNCTION(BlueprintCallable, Category = "Screens")
	void RestartLevel();

	UFUNCTION(BlueprintCallable, Category = "Screens")
	void QuitToMainMenu();

	/**
	 * Ends the run as a win: stops the player, makes them immune to further hits, and shows the victory screen
	 * after Delay seconds (e.g. to let an extraction camera shot play first).
	 */
	UFUNCTION(BlueprintCallable, Category = "Screens", meta = (AdvancedDisplay = "Delay"))
	void ShowVictory(float Delay = 2.0f);

	/** Shows a short hint banner near the bottom of the screen for Duration seconds, fading out at the end */
	UFUNCTION(BlueprintCallable, Category = "Screens")
	void ShowMessage(const FText& Message, float Duration = 3.0f);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY()
	TObjectPtr<UPlayerHealthComponent> Health;

	UPROPERTY()
	TObjectPtr<UInputComponent> ScreensInput;

	UPROPERTY()
	TObjectPtr<UUserWidget> DamageOverlay;

	UPROPERTY()
	TObjectPtr<UUserWidget> PauseMenu;

	UPROPERTY()
	TObjectPtr<UUserWidget> GameOverScreen;

	UPROPERTY()
	TObjectPtr<UUserWidget> VictoryScreen;

	UPROPERTY()
	TObjectPtr<UUserWidget> MessageBanner;

	UPROPERTY()
	TObjectPtr<UUserWidget> GameplayHUD;

	UPROPERTY()
	TObjectPtr<UUserWidget> Crosshair;

	UPROPERTY()
	TObjectPtr<class UArsenalInventoryComponent> Inventory;

	UPROPERTY()
	TObjectPtr<UUserWidget> StaminaWheel;

	UPROPERTY()
	TObjectPtr<class UPlayerStaminaComponent> Stamina;

	/** Seconds the stamina has been full, used to fade the wheel out */
	float StaminaFullTime = 0.0f;

	float MessageTimeRemaining = 0.0f;
	FTimerHandle VictoryTimer;

	bool bPaused = false;
	/** True once the run has ended either way (died or escaped); blocks pausing and further deaths */
	bool bGameOver = false;

	// hit snap: an offset layered on the control rotation and removed again, so the player's own aiming is untouched
	bool bHitKickActive = false;
	float HitKickElapsed = 0.0f;
	FRotator HitKickStartOffset = FRotator::ZeroRotator;
	FRotator HitKickAppliedOffset = FRotator::ZeroRotator;

	// death fall, animated in TickComponent until the game over screen pauses the game
	bool bPlayingDeathFall = false;
	float DeathFallElapsed = 0.0f;
	FRotator DeathStartRotation = FRotator::ZeroRotator;
	FVector DeathCameraStartLocation = FVector::ZeroVector;

	UPROPERTY()
	TObjectPtr<UCameraComponent> DeathCamera;

	FTimerHandle GameOverTimer;

	UFUNCTION()
	void OnOwnerControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	UFUNCTION()
	void OnPlayerDied(AActor* Attacker);

	UFUNCTION()
	void OnPlayerDamaged(AActor* Attacker);

	void UpdateHitKick(float DeltaTime);
	void SetHitKickOffset(const FRotator& NewOffset);

	void StartDeathFall();
	void UpdateDeathFall(float DeltaTime);

	void SetupForController(APlayerController* NewPlayerController);
	void ShowGameOver();
	void DisplayVictoryScreen();
	void StopPlayerForMatchEnd();
	void DebugHurt();

	UUserWidget* CreateScreen(TSubclassOf<UUserWidget> WidgetClass);
	void SetMenuInput(UUserWidget* FocusWidget, bool bGameStillReceivesInput);
	void SetGameplayInput();
};
