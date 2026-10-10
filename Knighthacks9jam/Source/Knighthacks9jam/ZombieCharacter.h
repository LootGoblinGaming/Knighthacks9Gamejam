// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieCharacter.generated.h"

UENUM(BlueprintType)
enum class EZombieState : uint8
{
    Patrol,
    Chase,
    Attack,
    Dead
};

UCLASS()
class KNIGHTHACKS9JAM_API   AZombieCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AZombieCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // State management
    UPROPERTY(BlueprintReadWrite, Category = "Zombie State")
    EZombieState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Settings")
    float DetectionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Settings")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Settings")
    float PatrolSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Settings")
    float ChaseSpeed = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Settings")
    float Health = 100.0f;

    void SetState(EZombieState NewState);
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

private:
    class AZombieAIController* ZombieController;
    class ACharacter* PlayerCharacter;
    FVector CurrentPatrolTarget;

    UPROPERTY()
	 UAnimInstance* ZombieAnimInstance = GetMesh()->GetAnimInstance();
};
