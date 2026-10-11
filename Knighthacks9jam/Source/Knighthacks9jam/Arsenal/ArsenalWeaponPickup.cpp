#include "ArsenalWeaponPickup.h"
#include "ArsenalWeaponBase.h"
#include "ArsenalInventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

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
	PickupTrigger->OnComponentBeginOverlap.AddDynamic(this, &AArsenalWeaponPickup::OnPickupTriggerBeginOverlap);
}

void AArsenalWeaponPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	RunningTime += DeltaSeconds;

	DisplayMesh->AddRelativeRotation(FRotator(0.0f, SpinSpeed * DeltaSeconds, 0.0f));

	if (BobPeriod > 0.0f)
	{
		const float BobOffset = FMath::Sin(RunningTime * UE_TWO_PI / BobPeriod) * BobHeight;
		DisplayMesh->SetRelativeLocation(DisplayBaseLocation + FVector(0.0f, 0.0f, BobOffset));
	}
}

void AArsenalWeaponPickup::OnPickupTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	UArsenalInventoryComponent* Inventory = OtherActor->FindComponentByClass<UArsenalInventoryComponent>();
	if (!Inventory || !Inventory->AddWeapon(WeaponClass))
	{
		return;
	}

	BP_OnPickedUp(OtherActor);
	Destroy();
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
