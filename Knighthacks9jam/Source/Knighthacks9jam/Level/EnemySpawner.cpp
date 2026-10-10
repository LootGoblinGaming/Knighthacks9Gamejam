#include "EnemySpawner.h"
#include "Components/SphereComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SpawnArea = CreateDefaultSubobject<USphereComponent>(TEXT("SpawnArea"));
	SpawnArea->SetupAttachment(RootComponent);
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnArea->SetHiddenInGame(true);
	SpawnArea->ShapeColor = FColor(255, 80, 40);
	SpawnArea->bIsEditorOnly = true;

	EditorIcon = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("EditorIcon"));
	if (EditorIcon)
	{
		EditorIcon->SetupAttachment(RootComponent);
	}
}

void AEnemySpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// keep the editor circle in sync with SpawnRadius as it's edited
	SpawnArea->SetSphereRadius(FMath::Max(SpawnRadius, 10.0f));
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bStartActive)
	{
		StartSpawning();
	}
}

void AEnemySpawner::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
}

void AEnemySpawner::StartSpawning()
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(SpawnTimer))
	{
		return;
	}

	// a 0 first delay means "spawn right away", which the timer can't express
	const float FirstDelay = InitialDelay > 0.0f ? InitialDelay : -1.0f;
	TimerManager.SetTimer(SpawnTimer, this, &AEnemySpawner::OnSpawnTimer, SpawnDelay, true, FirstDelay);
	if (InitialDelay <= 0.0f)
	{
		OnSpawnTimer();
	}
}

void AEnemySpawner::StopSpawning()
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
}

int32 AEnemySpawner::GetAliveCount() const
{
	int32 Alive = 0;
	for (const TWeakObjectPtr<APawn>& Enemy : SpawnedEnemies)
	{
		if (Enemy.IsValid())
		{
			Alive++;
		}
	}
	return Alive;
}

void AEnemySpawner::OnSpawnTimer()
{
	if (TotalSpawnLimit > 0 && TotalSpawned >= TotalSpawnLimit)
	{
		// spent - nothing left to do
		StopSpawning();
		return;
	}

	if (IsPlayerInRange())
	{
		SpawnEnemy();
	}
}

APawn* AEnemySpawner::SpawnEnemy()
{
	if (!SpawnClass)
	{
		return nullptr;
	}

	SpawnedEnemies.RemoveAll([](const TWeakObjectPtr<APawn>& Enemy) { return !Enemy.IsValid(); });
	if (SpawnedEnemies.Num() >= MaxAlive || (TotalSpawnLimit > 0 && TotalSpawned >= TotalSpawnLimit))
	{
		return nullptr;
	}

	FVector SpawnLocation;
	if (!FindSpawnLocation(SpawnLocation))
	{
		return nullptr;
	}

	// nav points sit on the floor; lift characters by their capsule so they don't start inside it
	if (const ACharacter* CharacterDefaults = Cast<ACharacter>(SpawnClass->GetDefaultObject()))
	{
		SpawnLocation.Z += CharacterDefaults->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}

	const FRotator SpawnRotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	APawn* Enemy = GetWorld()->SpawnActor<APawn>(SpawnClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!Enemy)
	{
		return nullptr;
	}

	// pawns only auto-possess "placed in world" by default, so spawned ones need their AI controller made explicitly
	if (!Enemy->GetController())
	{
		Enemy->SpawnDefaultController();
	}

	SpawnedEnemies.Add(Enemy);
	TotalSpawned++;
	OnEnemySpawned.Broadcast(Enemy);
	return Enemy;
}

bool AEnemySpawner::IsPlayerInRange() const
{
	if (ActivationRange <= 0.0f)
	{
		return true;
	}

	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	return Player && FVector::Dist(Player->GetActorLocation(), GetActorLocation()) <= ActivationRange;
}

bool AEnemySpawner::FindSpawnLocation(FVector& OutLocation) const
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	// a few attempts to find a walkable point that isn't right next to the player
	for (int32 Attempt = 0; Attempt < 8; Attempt++)
	{
		FVector Candidate = GetActorLocation();

		if (NavSystem)
		{
			FNavLocation NavLocation;
			const bool bFound = SpawnRadius > 0.0f
				? NavSystem->GetRandomReachablePointInRadius(GetActorLocation(), SpawnRadius, NavLocation)
				: NavSystem->ProjectPointToNavigation(GetActorLocation(), NavLocation);
			if (!bFound)
			{
				continue;
			}
			Candidate = NavLocation.Location;
		}

		if (!Player || FVector::Dist(Player->GetActorLocation(), Candidate) >= MinDistanceFromPlayer)
		{
			OutLocation = Candidate;
			return true;
		}
	}

	return false;
}
