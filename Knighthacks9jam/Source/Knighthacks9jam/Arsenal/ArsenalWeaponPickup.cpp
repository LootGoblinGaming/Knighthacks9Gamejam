#include "ArsenalWeaponPickup.h"
#include "ArsenalWeaponBase.h"
#include "ArsenalInventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ArsenalEffects.h"

AArsenalWeaponPickup::AArsenalWeaponPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = PickupTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("PickupTrigger"));
	PickupTrigger->SetSphereRadius(75.0f);

	// overlap Pawns only, never block anything
	PickupTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupTrigger->SetGenerateOverlapEvents(true);

	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	DisplayMesh->SetupAttachment(RootComponent);
	DisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(RootComponent);
	GlowLight->SetRelativeLocation(FVector(0.0f, 0.0f, -20.0f));
	GlowLight->SetIntensityUnits(ELightUnits::Candelas);
	GlowLight->SetIntensity(15.0f);
	GlowLight->SetAttenuationRadius(300.0f);
	GlowLight->SetLightColor(FLinearColor(0.35f, 1.0f, 0.8f));
	GlowLight->SetCastShadows(false);
}

void AArsenalWeaponPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// runs in the editor too, so the right gun shows as soon as WeaponClass is picked
	CopyWeaponMesh();
}

void AArsenalWeaponPickup::BeginPlay()
{
	Super::BeginPlay();

	DisplayBaseLocation = DisplayMesh->GetRelativeLocation();
	DisplayBaseScale = DisplayMesh->GetRelativeScale3D();
	GlowBaseIntensity = GlowLight->Intensity;
	PickupTrigger->OnComponentBeginOverlap.AddDynamic(this, &AArsenalWeaponPickup::OnPickupTriggerBeginOverlap);
}

void AArsenalWeaponPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	RunningTime += DeltaSeconds;

	if (bCollected)
	{
		PopElapsed += DeltaSeconds;
		const float Alpha = PopDuration > 0.0f ? FMath::Clamp(PopElapsed / PopDuration, 0.0f, 1.0f) : 1.0f;

		// swell to 140% over the first third, then shrink to nothing
		const float PopScale = Alpha < 0.33f ? FMath::Lerp(1.0f, 1.4f, Alpha / 0.33f) : FMath::Lerp(1.4f, 0.0f, (Alpha - 0.33f) / 0.67f);
		DisplayMesh->SetRelativeScale3D(DisplayBaseScale * PopScale);
		DisplayMesh->AddRelativeRotation(FRotator(0.0f, SpinSpeed * 4.0f * DeltaSeconds, 0.0f));
		GlowLight->SetIntensity(GlowBaseIntensity * (1.0f - Alpha));

		if (Alpha >= 1.0f)
		{
			Destroy();
		}
		return;
	}

	GlowLight->SetIntensity(GlowBaseIntensity * (1.0f - GlowPulseAmount * 0.5f * (1.0f - FMath::Cos(RunningTime * UE_TWO_PI / FMath::Max(BobPeriod, 0.1f)))));

	DisplayMesh->AddRelativeRotation(FRotator(0.0f, SpinSpeed * DeltaSeconds, 0.0f));

	if (BobPeriod > 0.0f)
	{
		const float BobOffset = FMath::Sin(RunningTime * UE_TWO_PI / BobPeriod) * BobHeight;
		DisplayMesh->SetRelativeLocation(DisplayBaseLocation + FVector(0.0f, 0.0f, BobOffset));
	}
}

void AArsenalWeaponPickup::OnPickupTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || bCollected)
	{
		return;
	}

	UArsenalInventoryComponent* Inventory = OtherActor->FindComponentByClass<UArsenalInventoryComponent>();
	if (!Inventory || !Inventory->AddWeapon(WeaponClass))
	{
		return;
	}

	bCollected = true;
	PickupTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	ArsenalEffects::SpawnEffectAtLocation(this, PickupEffect, DisplayMesh->GetComponentLocation(), FRotator::ZeroRotator);
	BP_OnPickedUp(OtherActor);

	// Tick plays the pop and destroys the pickup when it finishes
}

void AArsenalWeaponPickup::CopyWeaponMesh()
{
	const AArsenalWeaponBase* WeaponDefaults = WeaponClass ? WeaponClass->GetDefaultObject<AArsenalWeaponBase>() : nullptr;
	const UStaticMeshComponent* SourceMesh = WeaponDefaults ? WeaponDefaults->GetWeaponMesh() : nullptr;

	if (!SourceMesh)
	{
		DisplayMesh->SetStaticMesh(nullptr);
		return;
	}

	DisplayMesh->SetStaticMesh(SourceMesh->GetStaticMesh());
	DisplayMesh->SetRelativeScale3D(SourceMesh->GetRelativeScale3D());

	for (int32 MaterialIndex = 0; MaterialIndex < SourceMesh->GetNumMaterials(); MaterialIndex++)
	{
		DisplayMesh->SetMaterial(MaterialIndex, SourceMesh->GetMaterial(MaterialIndex));
	}
}
