#include "ArsenalWeaponBase.h"
#include "ArsenalProjectileBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"

AArsenalWeaponBase::AArsenalWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	WeaponMesh->SetCollisionProfileName(FName("NoCollision"));

	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	MuzzlePoint->SetupAttachment(WeaponMesh);
	MuzzlePoint->SetRelativeLocation(FVector(50.0f, 0.0f, 0.0f));
}

void AArsenalWeaponBase::StartFiring()
{
	bIsFiring = true;
	Fire();
}

void AArsenalWeaponBase::StopFiring()
{
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
}

FVector AArsenalWeaponBase::GetAimDirection_Implementation() const
{
	return MuzzlePoint->GetForwardVector();
}

void AArsenalWeaponBase::Fire()
{
	if (!bIsFiring || !ProjectileClass)
	{
		return;
	}

	const FVector SpawnLocation = MuzzlePoint->GetComponentLocation();
	const FRotator SpawnRotation = GetAimDirection().Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());

	GetWorld()->SpawnActor<AArsenalProjectileBase>(ProjectileClass, FTransform(SpawnRotation, SpawnLocation), SpawnParams);

	if (bFullAuto)
	{
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AArsenalWeaponBase::Fire, RefireRate, false);
	}
	else
	{
		StopFiring();
	}
}
