#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArsenalInventoryComponent.generated.h"

class AArsenalWeaponBase;

/**
 * Add this to the player Character to equip and fire the arsenal weapons. Spawns every entry
 * in WeaponClasses on BeginPlay, attached to the owner's mesh, accepts more at runtime through
 * AddWeapon() (e.g. from AArsenalWeaponPickup), and keeps exactly one visible and able to fire at a time.
 */
UCLASS(ClassGroup = (Arsenal), meta = (BlueprintSpawnableComponent))
class KNIGHTHACKS9JAM_API UArsenalInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UArsenalInventoryComponent();

	/** Weapons the owner starts with, in slot order. Index 0 is equipped by default. Leave empty to start unarmed. */
	UPROPERTY(EditAnywhere, Category = "Arsenal")
	TArray<TSubclassOf<AArsenalWeaponBase>> WeaponClasses;

	/** Socket on the owner's mesh to attach weapons to. Leave None to attach at the mesh's origin. */
	UPROPERTY(EditAnywhere, Category = "Arsenal")
	FName WeaponAttachSocket = NAME_None;

	UFUNCTION(BlueprintCallable, Category = "Arsenal")
	void EquipWeaponByIndex(int32 Index);

	/** Spawns a weapon of this class into the next slot, optionally equipping it. Returns false if it's already owned. */
	UFUNCTION(BlueprintCallable, Category = "Arsenal")
	bool AddWeapon(TSubclassOf<AArsenalWeaponBase> WeaponClass, bool bEquip = true);

	UFUNCTION(BlueprintPure, Category = "Arsenal")
	bool HasWeapon(TSubclassOf<AArsenalWeaponBase> WeaponClass) const;

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

private:
	/** Spawns the weapon hidden and attached to the owner. Returns nullptr on failure. */
	AArsenalWeaponBase* SpawnWeapon(TSubclassOf<AArsenalWeaponBase> WeaponClass);
};
