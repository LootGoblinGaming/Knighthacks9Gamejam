// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GoblinZombieCharacter.generated.h"
UENUM(BlueprintType)
enum class EGoblinZombieState : uint8
{
	Patrol,
	Chase,
	Attack,
	Dead
};

UCLASS()
class KNIGHTHACKS9JAM_API AGoblinZombieCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGoblinZombieCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	
protected:
	// Called when the game starts or when spawned


public:	
	// State management
	UPROPERTY(BlueprintReadWrite, Category = "Goblin state")
	EGoblinZombieState CurrentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goblin Settings")
	float DetectionRange = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goblin	 Settings")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goblin Settings")
	float PatrolSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goblin Settings")
	float ChaseSpeed = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goblin Settings")
	float Health = 100.0f;

	// Owns MaxWalkSpeed so arsenal slows/freezes/vaccine cure survive state changes - set speeds through it
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goblin Settings")
	class UMovementModifierComponent* MovementModifier;

	void SetState(EGoblinZombieState NewState);
	void UpdateAIState();

	// Patrol
	void Patrol();
	void SelectRandomPatrolPoint();

	// Chase
	void ChasePlayer();

	// Attack
	void AttackPlayer();

	// Health
	using Super::TakeDamage; // keep AActor::TakeDamage visible; Mac clang errors on hiding it
	void TakeDamage(float Damage);
	void Die();

	class AGoblinAIController* GoblinController;
	class ACharacter* PlayerCharacter;
	FVector CurrentPatrolTarget;

	UPROPERTY()
	UAnimInstance* ZombieAnimInstance = nullptr;
};
