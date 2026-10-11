#include "KillVolume.h"
#include "PlayerHealthComponent.h"
#include "Components/BoxComponent.h"

AKillVolume::AKillVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = KillBox = CreateDefaultSubobject<UBoxComponent>(TEXT("KillBox"));
	KillBox->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
	KillBox->ShapeColor = FColor::Red;
	KillBox->SetHiddenInGame(true);

	// overlap Pawns only, never block anything
	KillBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	KillBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	KillBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	KillBox->SetGenerateOverlapEvents(true);
}

void AKillVolume::BeginPlay()
{
	Super::BeginPlay();

	KillBox->OnComponentBeginOverlap.AddDynamic(this, &AKillVolume::OnKillBoxBeginOverlap);
}

void AKillVolume::OnKillBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (UPlayerHealthComponent* Health = OtherActor ? OtherActor->FindComponentByClass<UPlayerHealthComponent>() : nullptr)
	{
		Health->Kill(this);
	}
}
