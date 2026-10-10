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

	/** Optional blood splatter image (with transparency) for the damage overlay; a red wash is used if empty */
	UPROPERTY(EditAnywhere, Category = "Screens")
	TObjectPtr<UTexture2D> BloodOverlayTexture;

	/** Keys that open/close the pause menu. Note: Escape also stops Play-In-Editor, so test with P in the editor. */
	UPROPERTY(EditAnywhere, Category = "Screens")
	TArray<FKey> PauseKeys;

	/** Level opened by the Main Menu buttons */
	UPROPERTY(EditAnywhere, Category = "Screens")
	FName MainMenuLevel = FName("MainMenu");

	/** Beat between the killing hit and the game over screen */
	UPROPERTY(EditAnywhere, Category = "Screens", meta = (ClampMin = 0, Units = "s"))
	float GameOverDelay = 1.5f;

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

	bool bPaused = false;
	bool bGameOver = false;

	FTimerHandle GameOverTimer;

	UFUNCTION()
	void OnOwnerControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	UFUNCTION()
	void OnPlayerDied(AActor* Attacker);

	void SetupForController(APlayerController* NewPlayerController);
	void ShowGameOver();
	void DebugHurt();

	UUserWidget* CreateScreen(TSubclassOf<UUserWidget> WidgetClass);
	void SetMenuInput(UUserWidget* FocusWidget, bool bGameStillReceivesInput);
	void SetGameplayInput();
};
