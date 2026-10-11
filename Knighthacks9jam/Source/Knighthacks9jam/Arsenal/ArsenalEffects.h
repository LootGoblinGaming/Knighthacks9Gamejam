#pragma once

#include "CoreMinimal.h"

class UNiagaraSystem;
class USceneComponent;

/**
 * Fire-and-forget Niagara helpers for arsenal feedback. Every spawned effect is force-stopped after
 * MaxLifetime seconds, so a looping system assigned as a placeholder can't keep running forever.
 * All functions do nothing when Effect is null, so unassigned effect slots are safe.
 */
namespace ArsenalEffects
{
	void SpawnEffectAtLocation(const UObject* WorldContextObject, UNiagaraSystem* Effect, const FVector& Location, const FRotator& Rotation, float MaxLifetime = 3.0f);

	void SpawnEffectAttached(UNiagaraSystem* Effect, USceneComponent* AttachTo, float MaxLifetime = 3.0f);
}
