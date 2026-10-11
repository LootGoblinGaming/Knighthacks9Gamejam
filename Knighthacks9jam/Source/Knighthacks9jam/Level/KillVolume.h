#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KillVolume.generated.h"

class UBoxComponent;

/**
 * Invisible box that instantly kills the player on entry (pits, deep water, out-of-bounds).
 * Resize it with Box Extent or by scaling the actor; only actors with a UPlayerHealthComponent are affected.
 */
UCLASS()
class KNIGHTHACKS9JAM_API AKillVolume : public AActor
{
	GENERATED_BODY()

public:
	AKillVolume();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> KillBox;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnKillBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
