#include "PlayerStaminaComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

UPlayerStaminaComponent::UPlayerStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	Stamina = MaxStamina;

	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		WalkSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;
	}

	// the pawn usually begins play before it's possessed, so wait for the controller if it isn't there yet
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		Pawn->ReceiveControllerChangedDelegate.AddDynamic(this, &UPlayerStaminaComponent::OnOwnerControllerChanged);
		SetupForController(Cast<APlayerController>(Pawn->GetController()));
	}
}

void UPlayerStaminaComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (PlayerController && SprintInput)
	{
		PlayerController->PopInputComponent(SprintInput);
	}
}

void UPlayerStaminaComponent::OnOwnerControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	SetupForController(Cast<APlayerController>(NewController));
}

void UPlayerStaminaComponent::SetupForController(APlayerController* NewPlayerController)
{
	if (PlayerController || !NewPlayerController || !NewPlayerController->IsLocalController())
	{
		return;
	}
	PlayerController = NewPlayerController;

	SprintInput = NewObject<UInputComponent>(PlayerController, TEXT("PlayerSprintInput"));
	SprintInput->RegisterComponent();
	SprintInput->BindKey(SprintKey, IE_Pressed, this, &UPlayerStaminaComponent::OnSprintPressed);
	SprintInput->BindKey(SprintKey, IE_Released, this, &UPlayerStaminaComponent::OnSprintReleased);
	PlayerController->PushInputComponent(SprintInput);
}

void UPlayerStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	// only burn stamina while actually running on the ground with controls enabled (not dead, not in the extraction shot)
	const bool bMoving = Character->GetVelocity().Size2D() > 10.0f;
	const bool bCanSprint = bSprintHeld && bMoving && !bExhausted && Character->InputEnabled()
		&& Character->GetCharacterMovement()->IsMovingOnGround();

	SetSprinting(bCanSprint && Stamina > 0.0f);

	if (bSprinting)
	{
		TimeSinceSprint = 0.0f;
		Stamina = FMath::Max(Stamina - DrainPerSecond * DeltaTime, 0.0f);
		if (Stamina <= 0.0f)
		{
			bExhausted = true;
			SetSprinting(false);
		}
		return;
	}

	TimeSinceSprint += DeltaTime;
	if (TimeSinceSprint >= RegenDelay)
	{
		Stamina = FMath::Min(Stamina + RegenPerSecond * DeltaTime, MaxStamina);
		if (bExhausted && GetStaminaFraction() >= RecoverFraction)
		{
			bExhausted = false;
		}
	}
}

void UPlayerStaminaComponent::SetSprinting(bool bNewSprinting)
{
	if (bSprinting == bNewSprinting)
	{
		return;
	}
	bSprinting = bNewSprinting;

	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bSprinting ? WalkSpeed * SprintSpeedMultiplier : WalkSpeed;
	}
}
