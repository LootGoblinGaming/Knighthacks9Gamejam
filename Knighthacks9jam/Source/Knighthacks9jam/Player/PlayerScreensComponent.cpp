#include "PlayerScreensComponent.h"
#include "PlayerHealthComponent.h"
#include "PlayerScreenWidgets.h"
#include "ArsenalInventoryComponent.h"
#include "PlayerStaminaComponent.h"
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
	VictoryClass = UVictoryWidget::StaticClass();
	MessageClass = UMessageToastWidget::StaticClass();
	CrosshairClass = UCrosshairWidget::StaticClass();
	StaminaWheelClass = UStaminaWheelWidget::StaticClass();
	PauseKeys = { EKeys::Escape, EKeys::P };
}

void UPlayerScreensComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = GetOwner()->FindComponentByClass<UPlayerHealthComponent>();
	Inventory = GetOwner()->FindComponentByClass<UArsenalInventoryComponent>();
	Stamina = GetOwner()->FindComponentByClass<UPlayerStaminaComponent>();
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
	GetWorld()->GetTimerManager().ClearTimer(VictoryTimer);

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

	Crosshair = CreateScreen(CrosshairClass);
	if (Crosshair)
	{
		Crosshair->SetVisibility(ESlateVisibility::Collapsed);
		Crosshair->AddToViewport(2);
	}

	if (Stamina)
	{
		StaminaWheel = CreateScreen(StaminaWheelClass);
		if (StaminaWheel)
		{
			StaminaWheel->SetRenderOpacity(0.0f);
			StaminaWheel->AddToViewport(2);
		}
	}

	GameplayHUD = CreateScreen(HUDClass);
	if (GameplayHUD)
	{
		GameplayHUD->AddToViewport(1);
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

	if (Crosshair)
	{
		const bool bHoldingWeapon = Inventory && Inventory->GetCurrentWeapon();
		const ESlateVisibility WantedVisibility = bHoldingWeapon && !bPaused && !bGameOver ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		if (Crosshair->GetVisibility() != WantedVisibility)
		{
			Crosshair->SetVisibility(WantedVisibility);
		}
	}

	if (StaminaWheel && Stamina)
	{
		const float Fraction = Stamina->GetStaminaFraction();
		if (UStaminaWheelWidget* Wheel = Cast<UStaminaWheelWidget>(StaminaWheel))
		{
			Wheel->SetStamina(Fraction, Stamina->IsExhausted());
		}

		// visible while in use, then fades out a moment after refilling, like BotW
		StaminaFullTime = Fraction >= 1.0f ? StaminaFullTime + DeltaTime : 0.0f;
		const float TargetOpacity = (bGameOver || bPaused || StaminaFullTime > 0.6f) ? 0.0f : 1.0f;
		StaminaWheel->SetRenderOpacity(FMath::FInterpTo(StaminaWheel->GetRenderOpacity(), TargetOpacity, DeltaTime, 8.0f));
	}

	if (MessageBanner && MessageTimeRemaining > 0.0f)
	{
		MessageTimeRemaining -= DeltaTime;

		// fade over the last half second
		MessageBanner->SetRenderOpacity(FMath::Clamp(MessageTimeRemaining / 0.5f, 0.0f, 1.0f));
		if (MessageTimeRemaining <= 0.0f)
		{
			MessageBanner->RemoveFromParent();
		}
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
		// sink straight down in world space: the camera rides a head socket whose local axes depend on the rig,
		// so a relative-Z offset would slide it sideways on some characters
		DeathCamera->SetRelativeLocation(DeathCameraStartLocation);
		DeathCamera->AddWorldOffset(FVector(0.0f, 0.0f, -DeathCameraDrop * Alpha));
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

	StopPlayerForMatchEnd();

	UGameplayStatics::PlaySound2D(this, DeathSound);
	StartDeathFall();

	GetWorld()->GetTimerManager().SetTimer(GameOverTimer, this, &UPlayerScreensComponent::ShowGameOver, FMath::Max(GameOverDelay, 0.01f), false);
}

void UPlayerScreensComponent::StopPlayerForMatchEnd()
{
	ResumeGame();

	if (Inventory)
	{
		Inventory->StopFiringCurrentWeapon();
	}

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Pawn->DisableInput(PlayerController);
	}

	if (MessageBanner)
	{
		MessageBanner->RemoveFromParent();
		MessageTimeRemaining = 0.0f;
	}
}

void UPlayerScreensComponent::ShowVictory(float Delay)
{
	if (bGameOver)
	{
		return;
	}
	// bGameOver also makes OnPlayerDied ignore any zombie hit that lands during the extraction shot
	bGameOver = true;

	StopPlayerForMatchEnd();

	if (Delay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(VictoryTimer, this, &UPlayerScreensComponent::DisplayVictoryScreen, Delay, false);
	}
	else
	{
		DisplayVictoryScreen();
	}
}

void UPlayerScreensComponent::DisplayVictoryScreen()
{
	VictoryScreen = CreateScreen(VictoryClass);
	if (!VictoryScreen)
	{
		return;
	}

	VictoryScreen->AddToViewport(10);
	UGameplayStatics::SetGamePaused(this, true);
	SetMenuInput(VictoryScreen, false);
}

void UPlayerScreensComponent::ShowMessage(const FText& Message, float Duration)
{
	if (bGameOver)
	{
		return;
	}

	if (!MessageBanner)
	{
		MessageBanner = CreateScreen(MessageClass);
	}
	if (!MessageBanner)
	{
		return;
	}

	if (UMessageToastWidget* Toast = Cast<UMessageToastWidget>(MessageBanner))
	{
		Toast->SetMessage(Message);
	}

	if (!MessageBanner->IsInViewport())
	{
		MessageBanner->AddToViewport(4);
	}
	MessageBanner->SetRenderOpacity(1.0f);
	MessageTimeRemaining = FMath::Max(Duration, 0.1f);
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
