#include "PlayerScreensComponent.h"
#include "PlayerHealthComponent.h"
#include "PlayerScreenWidgets.h"
#include "ArsenalInventoryComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

UPlayerScreensComponent::UPlayerScreensComponent()
{
	// ticks to fade the damage overlay and animate the death fall
	PrimaryComponentTick.bCanEverTick = true;

	DamageOverlayClass = UDamageOverlayWidget::StaticClass();
	PauseMenuClass = UPauseMenuWidget::StaticClass();
	GameOverClass = UGameOverWidget::StaticClass();
	PauseKeys = { EKeys::Escape, EKeys::P };
}

void UPlayerScreensComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = GetOwner()->FindComponentByClass<UPlayerHealthComponent>();
	if (Health)
	{
		Health->OnDamaged.AddDynamic(this, &UPlayerScreensComponent::OnPlayerDamaged);
		Health->OnDied.AddDynamic(this, &UPlayerScreensComponent::OnPlayerDied);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerScreensComponent on %s found no PlayerHealthComponent; damage overlay and game over are disabled"), *GetOwner()->GetName());
	}

	// the pawn usually begins play before it's possessed, so wait for the controller if it isn't there yet
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Pawn->ReceiveControllerChangedDelegate.AddDynamic(this, &UPlayerScreensComponent::OnOwnerControllerChanged);
		SetupForController(Cast<APlayerController>(Pawn->GetController()));
	}
}

void UPlayerScreensComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(GameOverTimer);

	if (PlayerController && ScreensInput)
	{
		PlayerController->PopInputComponent(ScreensInput);
	}
}

void UPlayerScreensComponent::OnOwnerControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	SetupForController(Cast<APlayerController>(NewController));
}

void UPlayerScreensComponent::SetupForController(APlayerController* NewPlayerController)
{
	if (PlayerController || !NewPlayerController || !NewPlayerController->IsLocalController())
	{
		return;
	}
	PlayerController = NewPlayerController;

	DamageOverlay = CreateScreen(DamageOverlayClass);
	if (DamageOverlay)
	{
		if (UDamageOverlayWidget* DamageOverlayWidget = Cast<UDamageOverlayWidget>(DamageOverlay))
		{
			DamageOverlayWidget->SetBloodTexture(BloodOverlayTexture);
		}
		DamageOverlay->SetRenderOpacity(0.0f);
		DamageOverlay->AddToViewport(0);
	}

	// own input component, so the pause key works without touching the character's input setup
	ScreensInput = NewObject<UInputComponent>(PlayerController, TEXT("PlayerScreensInput"));
	ScreensInput->RegisterComponent();
	for (const FKey& PauseKey : PauseKeys)
	{
		ScreensInput->BindKey(PauseKey, IE_Pressed, this, &UPlayerScreensComponent::TogglePause).bExecuteWhenPaused = true;
	}
#if !UE_BUILD_SHIPPING
	if (DebugHurtKey.IsValid())
	{
		ScreensInput->BindKey(DebugHurtKey, IE_Pressed, this, &UPlayerScreensComponent::DebugHurt);
	}
#endif
	PlayerController->PushInputComponent(ScreensInput);
}

void UPlayerScreensComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DamageOverlay && Health)
	{
		DamageOverlay->SetRenderOpacity(Health->GetDamageOverlayAlpha());
	}

	if (bHitKickActive)
	{
		UpdateHitKick(DeltaTime);
	}

	if (bPlayingDeathFall)
	{
		UpdateDeathFall(DeltaTime);
	}
}

void UPlayerScreensComponent::OnPlayerDamaged(AActor* Attacker)
{
	UGameplayStatics::PlaySound2D(this, HitSound);

	if (!PlayerController)
	{
		return;
	}

	const float KickAngle = FMath::FRandRange(0.0f, UE_TWO_PI);
	HitKickStartOffset = FRotator(FMath::Sin(KickAngle) * HitViewKick, FMath::Cos(KickAngle) * HitViewKick, 0.0f);
	HitKickElapsed = 0.0f;
	bHitKickActive = true;
	SetHitKickOffset(HitKickStartOffset);

	if (HitCameraShake)
	{
		PlayerController->ClientStartCameraShake(HitCameraShake);
	}
}

void UPlayerScreensComponent::UpdateHitKick(float DeltaTime)
{
	HitKickElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(HitKickElapsed / HitKickRecoverTime, 0.0f, 1.0f);

	// quick at first, then settling gently onto the original aim
	SetHitKickOffset(HitKickStartOffset * FMath::Square(1.0f - Alpha));

	if (Alpha >= 1.0f)
	{
		bHitKickActive = false;
	}
}

void UPlayerScreensComponent::SetHitKickOffset(const FRotator& NewOffset)
{
	if (!PlayerController)
	{
		return;
	}

	// swap the previous offset for the new one; any mouse movement since then stays in the control rotation
	PlayerController->SetControlRotation(PlayerController->GetControlRotation() - HitKickAppliedOffset + NewOffset);
	HitKickAppliedOffset = NewOffset;
}

void UPlayerScreensComponent::StartDeathFall()
{
	if (!PlayerController)
	{
		return;
	}

	// drop any unfinished hit snap so the fall starts from where the player was really looking
	if (bHitKickActive)
	{
		SetHitKickOffset(FRotator::ZeroRotator);
		bHitKickActive = false;
	}

	bPlayingDeathFall = true;
	DeathFallElapsed = 0.0f;
	DeathStartRotation = PlayerController->GetControlRotation();
	DeathStartRotation.Pitch = FRotator::NormalizeAxis(DeathStartRotation.Pitch);

	DeathCamera = GetOwner()->FindComponentByClass<UCameraComponent>();
	if (DeathCamera)
	{
		DeathCameraStartLocation = DeathCamera->GetRelativeLocation();
	}

	if (PlayerController->PlayerCameraManager)
	{
		PlayerController->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, FMath::Max(GameOverDelay, 0.01f), DeathFadeColor, false, true);
	}
}

void UPlayerScreensComponent::UpdateDeathFall(float DeltaTime)
{
	DeathFallElapsed += DeltaTime;
	const float Duration = FMath::Max(GameOverDelay, 0.01f);

	// fast at first, settling at the end, like a body hitting the floor
	const float Alpha = FMath::InterpEaseOut(0.0f, 1.0f, FMath::Clamp(DeathFallElapsed / Duration, 0.0f, 1.0f), 2.0f);

	FRotator FallRotation = DeathStartRotation;
	FallRotation.Pitch = FMath::Max(DeathStartRotation.Pitch - DeathPitchDrop * Alpha, -85.0f);
	FallRotation.Roll = DeathTiltRoll * Alpha;
	PlayerController->SetControlRotation(FallRotation);

	if (DeathCamera)
	{
		DeathCamera->SetRelativeLocation(DeathCameraStartLocation - FVector(0.0f, 0.0f, DeathCameraDrop * Alpha));
	}

	if (Alpha >= 1.0f)
	{
		bPlayingDeathFall = false;
	}
}

void UPlayerScreensComponent::TogglePause()
{
	if (bPaused)
	{
		ResumeGame();
	}
	else
	{
		PauseGame();
	}
}

void UPlayerScreensComponent::PauseGame()
{
	if (bPaused || bGameOver || !PlayerController)
	{
		return;
	}

	if (!PauseMenu)
	{
		PauseMenu = CreateScreen(PauseMenuClass);
	}
	if (!PauseMenu)
	{
		return;
	}

	bPaused = true;
	PauseMenu->AddToViewport(5);
	UGameplayStatics::SetGamePaused(this, true);

	// game still gets input so the pause key can close the menu again
	SetMenuInput(PauseMenu, true);
}

void UPlayerScreensComponent::ResumeGame()
{
	if (!bPaused)
	{
		return;
	}

	bPaused = false;
	if (PauseMenu)
	{
		PauseMenu->RemoveFromParent();
	}
	UGameplayStatics::SetGamePaused(this, false);
	SetGameplayInput();
}

void UPlayerScreensComponent::RestartLevel()
{
	// input mode and cursor outlive the level, so reset them before travelling
	SetGameplayInput();
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}

void UPlayerScreensComponent::QuitToMainMenu()
{
	SetGameplayInput();
	UGameplayStatics::OpenLevel(this, MainMenuLevel);
}

void UPlayerScreensComponent::OnPlayerDied(AActor* Attacker)
{
	if (bGameOver)
	{
		return;
	}
	bGameOver = true;

	ResumeGame();

	if (UArsenalInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UArsenalInventoryComponent>())
	{
		Inventory->StopFiringCurrentWeapon();
	}

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Pawn->DisableInput(PlayerController);
	}

	UGameplayStatics::PlaySound2D(this, DeathSound);
	StartDeathFall();

	GetWorld()->GetTimerManager().SetTimer(GameOverTimer, this, &UPlayerScreensComponent::ShowGameOver, FMath::Max(GameOverDelay, 0.01f), false);
}

void UPlayerScreensComponent::ShowGameOver()
{
	GameOverScreen = CreateScreen(GameOverClass);
	if (!GameOverScreen)
	{
		return;
	}

	GameOverScreen->AddToViewport(10);
	UGameplayStatics::SetGamePaused(this, true);
	SetMenuInput(GameOverScreen, false);
}

void UPlayerScreensComponent::DebugHurt()
{
	if (Health)
	{
		Health->TakeHit(nullptr);
	}
}

UUserWidget* UPlayerScreensComponent::CreateScreen(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!PlayerController || !WidgetClass)
	{
		return nullptr;
	}

	UUserWidget* Screen = CreateWidget<UUserWidget>(PlayerController, WidgetClass);
	if (UPlayerMenuWidget* Menu = Cast<UPlayerMenuWidget>(Screen))
	{
		Menu->SetScreensComponent(this);
	}
	return Screen;
}

void UPlayerScreensComponent::SetMenuInput(UUserWidget* FocusWidget, bool bGameStillReceivesInput)
{
	if (!PlayerController)
	{
		return;
	}

	if (bGameStillReceivesInput)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputMode);
	}
	else
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
	}
	PlayerController->SetShowMouseCursor(true);
}

void UPlayerScreensComponent::SetGameplayInput()
{
	if (!PlayerController)
	{
		return;
	}

	PlayerController->SetInputMode(FInputModeGameOnly());
	PlayerController->SetShowMouseCursor(false);
}
