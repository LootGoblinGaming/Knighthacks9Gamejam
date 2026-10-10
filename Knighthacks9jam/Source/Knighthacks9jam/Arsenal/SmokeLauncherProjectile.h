#pragma once

#include "CoreMinimal.h"
#include "ArsenalProjectileBase.h"
#include "SmokeLauncherProjectile.generated.h"

class USphereComponent;
class UNiagaraSystem;

/**
 * Smoke Launcher round. On impact, spawns a smoke VFX cloud and a collision volume
 * that blocks the Visibility trace channel, breaking AIPerception sight lines and
 * forcing zombies into their Investigate state.
 */
UCLASS()
class KNIGHTHACKS9JAM_API ASmokeLauncherProjectile : public AArsenalProjectileBase
{
	GENERATED_BODY()

public:
	ASmokeLauncherProjectile();

protected:
	/** Blocks the Visibility trace channel only, so it occludes AI sight without physically shoving pawns */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> VisionBlocker;

	UPROPERTY(EditAnywhere, Category = "Smoke")
	TObjectPtr<UNiagaraSystem> SmokeEffect;

	UPROPERTY(EditAnywhere, Category = "Smoke", meta = (ClampMin = 50, ClampMax = 2000, Units = "cm"))
	float SmokeRadius = 400.0f;

	virtual void OnProjectileImpact(const FHitResult& Hit) override;
};
