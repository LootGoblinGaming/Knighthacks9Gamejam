#include "ArsenalInventoryComponent.h"
#include "ArsenalWeaponBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UArsenalInventoryComponent::UArsenalInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UArsenalInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	for (TSubclassOf<AArsenalWeaponBase> WeaponClass : WeaponClasses)
	{
		if (AArsenalWeaponBase* Weapon = SpawnWeapon(WeaponClass))
		{
			SpawnedWeapons.Add(Weapon);
		}
	}

	if (SpawnedWeapons.Num() > 0)
	{
		EquipWeaponByIndex(0);
	}
}

AArsenalWeaponBase* UArsenalInventoryComponent::SpawnWeapon(TSubclassOf<AArsenalWeaponBase> WeaponClass)
{
	if (!WeaponClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());

	AArsenalWeaponBase* Weapon = GetWorld()->SpawnActor<AArsenalWeaponBase>(WeaponClass, SpawnParams);
	if (!Weapon)
	{
		return nullptr;
	}

	USceneComponent* AttachTarget = nullptr;
	if (ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner()))
	{
		AttachTarget = OwningCharacter->GetMesh();
	}
	else if (AActor* OwnerActor = GetOwner())
	{
		AttachTarget = OwnerActor->GetRootComponent();
	}

	if (AttachTarget)
	{
		Weapon->AttachToComponent(AttachTarget, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocket);
	}

	Weapon->SetActorHiddenInGame(true);
	return Weapon;
}

bool UArsenalInventoryComponent::AddWeapon(TSubclassOf<AArsenalWeaponBase> WeaponClass, bool bEquip)
{
	if (!WeaponClass || HasWeapon(WeaponClass))
	{
		return false;
	}

	AArsenalWeaponBase* Weapon = SpawnWeapon(WeaponClass);
	if (!Weapon)
	{
		return false;
	}

	const int32 NewIndex = SpawnedWeapons.Add(Weapon);

	// an unarmed owner always equips its first weapon
	if (bEquip || !SpawnedWeapons.IsValidIndex(CurrentWeaponIndex))
	{
		EquipWeaponByIndex(NewIndex);
	}
	return true;
}

bool UArsenalInventoryComponent::HasWeapon(TSubclassOf<AArsenalWeaponBase> WeaponClass) const
{
	for (const AArsenalWeaponBase* Weapon : SpawnedWeapons)
	{
		if (Weapon && Weapon->GetClass() == WeaponClass)
		{
			return true;
		}
	}
	return false;
}

void UArsenalInventoryComponent::EquipWeaponByIndex(int32 Index)
{
	if (!SpawnedWeapons.IsValidIndex(Index))
	{
		return;
	}

	if (SpawnedWeapons.IsValidIndex(CurrentWeaponIndex))
	{
		SpawnedWeapons[CurrentWeaponIndex]->StopFiring();
		SpawnedWeapons[CurrentWeaponIndex]->SetActorHiddenInGame(true);
	}

	CurrentWeaponIndex = Index;
	SpawnedWeapons[CurrentWeaponIndex]->SetActorHiddenInGame(false);
}

void UArsenalInventoryComponent::EquipNextWeapon()
{
	if (SpawnedWeapons.Num() == 0)
	{
		return;
	}

	const int32 NextIndex = (CurrentWeaponIndex + 1) % SpawnedWeapons.Num();
	EquipWeaponByIndex(NextIndex);
}

void UArsenalInventoryComponent::EquipPreviousWeapon()
{
	if (SpawnedWeapons.Num() == 0)
	{
		return;
	}

	const int32 PrevIndex = (CurrentWeaponIndex - 1 + SpawnedWeapons.Num()) % SpawnedWeapons.Num();
	EquipWeaponByIndex(PrevIndex);
}

void UArsenalInventoryComponent::StartFiringCurrentWeapon()
{
	if (AArsenalWeaponBase* Current = GetCurrentWeapon())
	{
		Current->StartFiring();
	}
}

void UArsenalInventoryComponent::StopFiringCurrentWeapon()
{
	if (AArsenalWeaponBase* Current = GetCurrentWeapon())
	{
		Current->StopFiring();
	}
}

AArsenalWeaponBase* UArsenalInventoryComponent::GetCurrentWeapon() const
{
	return SpawnedWeapons.IsValidIndex(CurrentWeaponIndex) ? SpawnedWeapons[CurrentWeaponIndex] : nullptr;
}
