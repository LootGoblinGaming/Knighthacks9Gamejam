#include "ArsenalEffects.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	void LimitLifetime(UNiagaraComponent* EffectComponent, float MaxLifetime, bool bCutOff = false)
	{
		if (!EffectComponent || MaxLifetime <= 0.0f)
		{
			return;
		}

		// Deactivate lets live particles finish, then the auto-destroying component cleans itself up
		TWeakObjectPtr<UNiagaraComponent> WeakComponent = EffectComponent;
		FTimerHandle UnusedHandle;
		EffectComponent->GetWorld()->GetTimerManager().SetTimer(UnusedHandle, FTimerDelegate::CreateLambda([WeakComponent, bCutOff]()
		{
			if (WeakComponent.IsValid())
			{
				if (bCutOff)
				{
					WeakComponent->DeactivateImmediate();
				}
				else
				{
					WeakComponent->Deactivate();
				}
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

void ArsenalEffects::SpawnEffectAttached(UNiagaraSystem* Effect, USceneComponent* AttachTo, float MaxLifetime, float Scale, bool bCutOffAtLifetime)
{
	if (!Effect || !AttachTo)
	{
		return;
	}

	UNiagaraComponent* EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(Effect, AttachTo, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
	if (EffectComponent)
	{
		// world scale, so the attach parent's own scale (e.g. a scaled-down gun mesh) doesn't compound it
		EffectComponent->SetWorldScale3D(FVector(FMath::Max(Scale, 0.01f)));

		// match the rendering mode of whatever it's attached to, so effects on first-person weapons line up with them
		for (USceneComponent* Parent = AttachTo; Parent; Parent = Parent->GetAttachParent())
		{
			if (const UPrimitiveComponent* ParentPrimitive = Cast<UPrimitiveComponent>(Parent))
			{
				EffectComponent->SetFirstPersonPrimitiveType(ParentPrimitive->FirstPersonPrimitiveType);
				break;
			}
		}
	}
	LimitLifetime(EffectComponent, MaxLifetime, bCutOffAtLifetime);
}
