#include "ArsenalWeaponBase.h"
#include "ArsenalProjectileBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/HitResult.h"
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "ArsenalEffects.h"

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

void AArsenalWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	OwningPawn = Cast<APawn>(GetOwner());
}

void AArsenalWeaponBase::StartFiring()
{
	bIsFiring = true;

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(RefireTimer))
	{
		return;
	}

	// still cooling down from the last shot: fire as soon as it's ready, if the trigger is still held by then
	const double CooldownRemaining = RefireRate - (GetWorld()->GetTimeSeconds() - LastFireTime);
	if (CooldownRemaining > 0.0)
	{
		TimerManager.SetTimer(RefireTimer, this, &AArsenalWeaponBase::Fire, CooldownRemaining, false);
		return;
	}

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

	LastFireTime = GetWorld()->GetTimeSeconds();

	if (bHitscanWeapon)
	{
		FireHitscan();
	}
	else
	{
		const FVector SpawnLocation = MuzzlePoint->GetComponentLocation();
		const FRotator SpawnRotation = GetAimDirection().Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = OwningPawn;

		GetWorld()->SpawnActor<AArsenalProjectileBase>(ProjectileClass, FTransform(SpawnRotation, SpawnLocation), SpawnParams);
	}

	PlayFireFeedback();

	if (bFullAuto)
	{
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AArsenalWeaponBase::Fire, RefireRate, false);
	}
	else
	{
		StopFiring();
	}
}

void AArsenalWeaponBase::PlayFireFeedback()
{
	UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzlePoint->GetComponentLocation());
	ArsenalEffects::SpawnEffectAttached(MuzzleFlashEffect, MuzzlePoint);

	APlayerController* PlayerController = OwningPawn ? Cast<APlayerController>(OwningPawn->GetController()) : nullptr;
	if (!PlayerController)
	{
		return;
	}

	FRotator ControlRotation = PlayerController->GetControlRotation();
	ControlRotation.Pitch += RecoilPitch;
	ControlRotation.Yaw += FMath::FRandRange(-RecoilYawJitter, RecoilYawJitter);
	PlayerController->SetControlRotation(ControlRotation);

	if (FireCameraShake)
	{
		PlayerController->ClientStartCameraShake(FireCameraShake);
	}
}

void AArsenalWeaponBase::FireHitscan()
{
	if (!OwningPawn)
	{
		return;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	OwningPawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * HitscanRange);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwningPawn);
	QueryParams.AddIgnoredActor(this);

	// Camera channel, not Visibility: the default Pawn collision preset ignores Visibility, so a Visibility trace passes through characters
	FHitResult Hit;
	const bool bHitSomething = GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Camera, QueryParams);

	if (!bHitSomething)
	{
		// nothing in range - treat the far end of the trace as the "impact" so the projectile still has somewhere to land
		Hit.Location = TraceEnd;
		Hit.ImpactPoint = TraceEnd;
		Hit.ImpactNormal = -EyeRotation.Vector();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = OwningPawn;

	AArsenalProjectileBase* ImpactActor = GetWorld()->SpawnActor<AArsenalProjectileBase>(ProjectileClass, FTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint), SpawnParams);
	if (ImpactActor)
	{
		ImpactActor->SimulateInstantHit(Hit);
	}
}
