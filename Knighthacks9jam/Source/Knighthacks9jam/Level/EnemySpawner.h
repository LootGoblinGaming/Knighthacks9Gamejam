#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class USphereComponent;
class UBillboardComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemySpawned, APawn*, SpawnedEnemy);

/**
 * Level design spawner. Drop it in a level, pick SpawnClass (e.g. BP_ZombieCharacter), and it spawns one enemy
 * every SpawnDelay seconds at a random walkable point within SpawnRadius, keeping at most MaxAlive of its enemies
 * in the level at once and stopping for good after TotalSpawnLimit. Optionally it only spawns while the player is
 * within ActivationRange, and never closer than MinDistanceFromPlayer, so enemies don't pop in in front of them.
 *
 * Every enemy it spawned that still exists counts toward MaxAlive - including cured zombies, so curing an area clears it.
 */
UCLASS()
class KNIGHTHACKS9JAM_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

	/** What to spawn. Must be a Pawn; it gets an AI controller automatically. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TSubclassOf<APawn> SpawnClass;

	/** Most enemies from this spawner that can exist at the same time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = 1))
	int32 MaxAlive = 3;

	/** Total enemies this spawner will ever create. 0 = no limit (keeps replacing enemies as they're removed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = 0))
	int32 TotalSpawnLimit = 0;

	/** Seconds between spawns */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = 0.1, Units = "s"))
	float SpawnDelay = 5.0f;

	/** Seconds after activation before the first spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = 0, Units = "s"))
	float InitialDelay = 0.0f;

	/** Enemies appear at a random walkable (NavMesh) point within this distance of the spawner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = 0, Units = "cm"))
	float SpawnRadius = 300.0f;

	/** Start spawning as soon as the level begins. Turn off to start it from a trigger with StartSpawning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	bool bStartActive = true;

	/** Only spawn while the player is within this distance. 0 = spawn regardless of where the player is. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Player", meta = (ClampMin = 0, Units = "cm"))
	float ActivationRange = 0.0f;

	/** Never spawn an enemy closer than this to the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Player", meta = (ClampMin = 0, Units = "cm"))
	float MinDistanceFromPlayer = 800.0f;

	/** Fired for every enemy this spawner creates */
	UPROPERTY(BlueprintAssignable, Category = "Spawner")
	FOnEnemySpawned OnEnemySpawned;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StopSpawning();

	/** Spawns one enemy now, if the limits allow. Returns the enemy, or null if nothing was spawned. */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	APawn* SpawnEnemy();

	UFUNCTION(BlueprintPure, Category = "Spawner")
	int32 GetAliveCount() const;

	UFUNCTION(BlueprintPure, Category = "Spawner")
	int32 GetTotalSpawned() const { return TotalSpawned; }

protected:
	/** Editor-only: shows the spawn area */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> SpawnArea;

	UPROPERTY()
	TObjectPtr<UBillboardComponent> EditorIcon;

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:
	TArray<TWeakObjectPtr<APawn>> SpawnedEnemies;
	int32 TotalSpawned = 0;
	FTimerHandle SpawnTimer;

	void OnSpawnTimer();
	bool IsPlayerInRange() const;
	bool FindSpawnLocation(FVector& OutLocation) const;
};
