#include "MovementModifierComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMovementModifierComponent::UMovementModifierComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMovementModifierComponent::BeginPlay()
{
	Super::BeginPlay();

	CaptureBaseSpeedIfUnset();
}

void UMovementModifierComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(FreezeTimer);
}

UMovementModifierComponent* UMovementModifierComponent::FindOrAdd(ACharacter* Character)
{
	if (!Character)
	{
		return nullptr;
	}

	if (UMovementModifierComponent* Existing = Character->FindComponentByClass<UMovementModifierComponent>())
	{
		return Existing;
	}

	UMovementModifierComponent* Added = NewObject<UMovementModifierComponent>(Character);
	Added->RegisterComponent();
	Added->CaptureBaseSpeedIfUnset();
	return Added;
}

void UMovementModifierComponent::SetBaseSpeed(float NewBaseSpeed)
{
	BaseSpeed = FMath::Max(NewBaseSpeed, 0.0f);
	ApplySpeed();
}

void UMovementModifierComponent::AddSlow(float Amount)
{
	CaptureBaseSpeedIfUnset();
	TotalSlow += FMath::Max(Amount, 0.0f);
	ApplySpeed();
}

void UMovementModifierComponent::Freeze(float Duration)
{
	CaptureBaseSpeedIfUnset();

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(FreezeTimer) && TimerManager.GetTimerRemaining(FreezeTimer) >= Duration)
	{
		return;
	}

	bFrozen = true;
	TimerManager.SetTimer(FreezeTimer, this, &UMovementModifierComponent::OnFreezeEnded, Duration, false);
	ApplySpeed();
}

void UMovementModifierComponent::LockSpeed(float LockedSpeed)
{
	bSpeedLocked = true;
	LockedSpeedValue = FMath::Max(LockedSpeed, 0.0f);
	ApplySpeed();
}

float UMovementModifierComponent::GetEffectiveSpeed() const
{
	if (bSpeedLocked)
	{
		return LockedSpeedValue;
	}

	if (bFrozen)
	{
		return 0.0f;
	}

	const float Base = FMath::Max(BaseSpeed, 0.0f);

	// a base speed already at or under the floor (e.g. 0 while attacking) is left alone
	if (Base <= MinSlowedSpeed)
	{
		return Base;
	}

	return FMath::Max(Base - TotalSlow, MinSlowedSpeed);
}

void UMovementModifierComponent::CaptureBaseSpeedIfUnset()
{
	if (BaseSpeed >= 0.0f)
	{
		return;
	}

	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		BaseSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;
	}
}

void UMovementModifierComponent::OnFreezeEnded()
{
	bFrozen = false;
	ApplySpeed();
}

void UMovementModifierComponent::ApplySpeed()
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = GetEffectiveSpeed();
	}
}
