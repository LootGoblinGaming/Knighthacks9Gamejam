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

	/**
	 * Scale multiplies the effect's size. bCutOffAtLifetime removes live particles at MaxLifetime instead of
	 * letting them fade out, for short bursts like muzzle flashes.
	 */
	void SpawnEffectAttached(UNiagaraSystem* Effect, USceneComponent* AttachTo, float MaxLifetime = 3.0f, float Scale = 1.0f, bool bCutOffAtLifetime = false);
}
