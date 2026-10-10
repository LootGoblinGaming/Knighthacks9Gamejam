#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArsenalInventoryComponent.generated.h"

class AArsenalWeaponBase;

/**
 * Add this to the player Character to equip and fire the arsenal weapons. Spawns every entry
 * in WeaponClasses on BeginPlay, attached to the owner's mesh, and keeps exactly one visible
 * and able to fire at a time.
 */
UCLASS(ClassGroup = (Arsenal), meta = (BlueprintSpawnableComponent))
class KNIGHTHACKS9JAM_API UArsenalInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UArsenalInventoryComponent();

	/** Weapon classes to spawn and equip, in slot order. Index 0 is equipped by default. */
	UPROPERTY(EditAnywhere, Category = "Arsenal")
	TArray<TSubclassOf<AArsenalWeaponBase>> WeaponClasses;

	/** Socket on the owner's mesh to attach weapons to. Leave None to attach at the mesh's origin. */
	UPROPERTY(EditAnywhere, Category = "Arsenal")
	FName WeaponAttachSocket = NAME_None;

	UFUNCTION(BlueprintCallable, Category = "Arsenal")
	void EquipWeaponByIndex(int32 Index);

	/** Equips the next weapon in WeaponClasses, wrapping around to the first after the last */
	UFUNCTION(BlueprintCallable, Category = "Arsenal")
	void EquipNextWeapon();

	/** Equips the previous weapon in WeaponClasses, wrapping around to the last after the first */
	UFUNCTION(BlueprintCallable, Category = "Arsenal")
	void EquipPreviousWeapon();

	UFUNCTION(BlueprintCallable, Category = "Arsenal")
	void StartFiringCurrentWeapon();

	UFUNCTION(BlueprintCallable, Category = "Arsenal")
	void StopFiringCurrentWeapon();

	UFUNCTION(BlueprintPure, Category = "Arsenal")
	AArsenalWeaponBase* GetCurrentWeapon() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TArray<TObjectPtr<AArsenalWeaponBase>> SpawnedWeapons;

	int32 CurrentWeaponIndex = -1;
};
