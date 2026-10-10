#pragma once

#include "CoreMinimal.h"
#include "ArsenalProjectileBase.h"
#include "BioFoamProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UNiagaraSystem;

/**
 * Bio-Foam Payload grenade. On impact, lays a foam zone for FoamLifetime seconds; every Character
 * inside it when it lands, or that walks into it while it lasts, is frozen for FreezeDuration
 * through its UMovementModifierComponent, then released at whatever speed it should have.
 */
UCLASS()
class KNIGHTHACKS9JAM_API ABioFoamProjectile : public AArsenalProjectileBase
{
	GENERATED_BODY()

public:
	ABioFoamProjectile();

protected:
	/** Overlap-only volume (Pawn channel) that applies the freeze; disabled until impact */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> FoamZone;

	/** Placeholder dome showing the foam area; swap the mesh/material in the Blueprint for real art */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FoamVisual;

	UPROPERTY(EditAnywhere, Category = "Foam")
	TObjectPtr<UNiagaraSystem> FoamEffect;

	UPROPERTY(EditAnywhere, Category = "Foam", meta = (ClampMin = 50, ClampMax = 2000, Units = "cm"))
	float FoamRadius = 300.0f;

	/** How long each Character stays frozen */
	UPROPERTY(EditAnywhere, Category = "Foam", meta = (ClampMin = 0.1, ClampMax = 30, Units = "s"))
	float FreezeDuration = 4.0f;

	/** How long the foam zone stays on the ground, freezing anything that walks in */
	UPROPERTY(EditAnywhere, Category = "Foam", meta = (ClampMin = 0.1, ClampMax = 30, Units = "s"))
	float FoamLifetime = 4.0f;

	virtual void BeginPlay() override;
	virtual void OnProjectileImpact(const FHitResult& Hit) override;

	UFUNCTION()
	void OnFoamZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void FreezeActor(AActor* Actor);
};
