// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieCharacter.h"
#include "ZombieAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MovementModifierComponent.h"
#include "Kismet/GameplayStatics.h"

AZombieCharacter::AZombieCharacter()
{
    PrimaryActorTick.TickInterval = 0.1f;
    GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
    MovementModifier = CreateDefaultSubobject<UMovementModifierComponent>(TEXT("MovementModifier"));
    // Rotate the character toward its movement direction.
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
}

void AZombieCharacter::BeginPlay()
{
    Super::BeginPlay();

    ZombieController = Cast<AZombieAIController>(Controller);
    PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	ZombieAnimInstance = GetMesh()->GetAnimInstance();
    
    SetState(EZombieState::Patrol);
    SelectRandomPatrolPoint();
}

void AZombieCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState == EZombieState::Dead)
        return;

    if (!PlayerCharacter)
        return;

    float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());

    // Transition logic
    if (DistanceToPlayer < DetectionRange && CurrentState != EZombieState::Attack)
    {
        SetState(EZombieState::Chase);
    }
    else if (DistanceToPlayer >= DetectionRange && CurrentState == EZombieState::Chase)
    {
        SetState(EZombieState::Patrol);
    }
    else if (DistanceToPlayer < AttackRange)
    {
        SetState(EZombieState::Attack);
    }

    UpdateAIState();
}

void AZombieCharacter::SetState(EZombieState NewState)
{
    if (CurrentState == NewState)
        return;

    CurrentState = NewState;

    switch (CurrentState)
    {
        case EZombieState::Patrol:
            MovementModifier->SetBaseSpeed(PatrolSpeed);
            break;
        case EZombieState::Chase:
            MovementModifier->SetBaseSpeed(ChaseSpeed);
            break;
        case EZombieState::Attack:
            MovementModifier->SetBaseSpeed(0.0f);
            break;
        case EZombieState::Dead:
            GetCharacterMovement()->DisableMovement();
            break;
    }
}

void AZombieCharacter::UpdateAIState()
{
    switch (CurrentState)
    {
        case EZombieState::Patrol:
            Patrol();
            break;
        case EZombieState::Chase:
            ChasePlayer();
            break;
        case EZombieState::Attack:
            AttackPlayer();
            break;
        case EZombieState::Dead:
            break;
    }
}

void AZombieCharacter::Patrol()
{
    if (!ZombieController)
        return;

    FVector CurrentLocation = GetActorLocation();
    float DistanceToTarget = FVector::Dist(CurrentLocation, CurrentPatrolTarget);

    if (DistanceToTarget < 100.0f)
    {
        SelectRandomPatrolPoint();
    }

    ZombieController->MoveToLocation(CurrentPatrolTarget, 10.0f);
}

void AZombieCharacter::SelectRandomPatrolPoint()
{
    FVector RandomOffset = FVector(FMath::FRand() - 0.5f, FMath::FRand() - 0.5f, 0.0f).GetSafeNormal() * 1000.0f;
    CurrentPatrolTarget = GetActorLocation() + RandomOffset;
}

void AZombieCharacter::ChasePlayer()
{
    if (!ZombieController || !PlayerCharacter)
        return;
	//movementspeed = ChaseSpeed;
    ZombieController->MoveToActor(PlayerCharacter, AttackRange);
}

void AZombieCharacter::AttackPlayer()
{
    if (!PlayerCharacter)
        return;

    // Simple attack - just face the player
    FVector DirectionToPlayer = (PlayerCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FRotator NewRotation = DirectionToPlayer.Rotation();
    SetActorRotation(NewRotation);

    // TODO: Play attack animation and deal damage
    UE_LOG(LogTemp, Warning, TEXT("Attacking Player!"));
}

void AZombieCharacter::TakeDamage(float Damage)
{
    Health -= Damage;

    if (Health <= 0.0f)
    {
        Die();
    }
}

void AZombieCharacter::Die()
{
    SetState(EZombieState::Dead);
    // TODO: Play death animation
}



