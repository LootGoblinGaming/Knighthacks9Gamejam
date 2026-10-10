#include "ArsenalEffects.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	void LimitLifetime(UNiagaraComponent* EffectComponent, float MaxLifetime)
	{
		if (!EffectComponent || MaxLifetime <= 0.0f)
		{
			return;
		}

		// Deactivate lets live particles finish, then the auto-destroying component cleans itself up
		TWeakObjectPtr<UNiagaraComponent> WeakComponent = EffectComponent;
		FTimerHandle UnusedHandle;
		EffectComponent->GetWorld()->GetTimerManager().SetTimer(UnusedHandle, FTimerDelegate::CreateLambda([WeakComponent]()
		{
			if (WeakComponent.IsValid())
			{
				WeakComponent->Deactivate();
			}
		}), MaxLifetime, false);
	}
}

void ArsenalEffects::SpawnEffectAtLocation(const UObject* WorldContextObject, UNiagaraSystem* Effect, const FVector& Location, const FRotator& Rotation, float MaxLifetime)
{
	if (!Effect || !WorldContextObject)
	{
		return;
	}

	LimitLifetime(UNiagaraFunctionLibrary::SpawnSystemAtLocation(WorldContextObject, Effect, Location, Rotation), MaxLifetime);
}

void ArsenalEffects::SpawnEffectAttached(UNiagaraSystem* Effect, USceneComponent* AttachTo, float MaxLifetime)
{
	if (!Effect || !AttachTo)
	{
		return;
	}

	LimitLifetime(UNiagaraFunctionLibrary::SpawnSystemAttached(Effect, AttachTo, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true), MaxLifetime);
}
