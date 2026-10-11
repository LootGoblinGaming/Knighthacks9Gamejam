#include "PlayerHealthComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"

UPlayerHealthComponent::UPlayerHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// lets zombies use the engine's standard ApplyDamage instead of knowing about this component
	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UPlayerHealthComponent::OnOwnerTakeAnyDamage);
	}
}

void UPlayerHealthComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(RecoveryTimer);
}

bool UPlayerHealthComponent::TakeHit(AActor* Attacker)
{
	const double Now = GetWorld()->GetTimeSeconds();
	if (bDead || Now - LastHitTime < HitInvulnerabilityTime)
	{
		return false;
	}
	LastHitTime = Now;

	if (bDamaged)
	{
		bDead = true;
		GetWorld()->GetTimerManager().ClearTimer(RecoveryTimer);
		OnDied.Broadcast(Attacker);
		return true;
	}

	bDamaged = true;
	GetWorld()->GetTimerManager().SetTimer(RecoveryTimer, this, &UPlayerHealthComponent::Recover, RecoveryTime, false);
	OnDamaged.Broadcast(Attacker);
	return true;
}

void UPlayerHealthComponent::Kill(AActor* Killer)
{
	if (bDead)
	{
		return;
	}

	bDead = true;
	GetWorld()->GetTimerManager().ClearTimer(RecoveryTimer);
	OnDied.Broadcast(Killer);
}

float UPlayerHealthComponent::GetDamageOverlayAlpha() const
{
	if (bDead)
	{
		return 1.0f;
	}

	if (!bDamaged || RecoveryTime <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(GetWorld()->GetTimerManager().GetTimerRemaining(RecoveryTimer) / RecoveryTime, 0.0f, 1.0f);
}

void UPlayerHealthComponent::OnOwnerTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage > 0.0f)
	{
		TakeHit(DamageCauser);
	}
}

void UPlayerHealthComponent::Recover()
{
	bDamaged = false;
	OnRecovered.Broadcast();
}
