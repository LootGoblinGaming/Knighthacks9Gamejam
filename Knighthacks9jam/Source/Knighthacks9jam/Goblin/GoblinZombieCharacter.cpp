// Fill out your copyright notice in the Description page of Project Settings.


#include "GoblinZombieCharacter.h"
#include "GoblinAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MovementModifierComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGoblinZombieCharacter::AGoblinZombieCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
	MovementModifier = CreateDefaultSubobject<UMovementModifierComponent>(TEXT("MovementModifier"));
	//rotate character toward the player
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
}

// Called when the game starts or when spawned
void AGoblinZombieCharacter::BeginPlay()
{
	Super::BeginPlay();
	GoblinController = Cast<AGoblinAIController>(Controller);
	PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	GoblinZombieAnimInstance = GetMesh()->GetAnimInstance();
	SetState(EGoblinZombieState::Patrol);
	SelectRandomPatrolPoint();
}

// Called every frame
void AGoblinZombieCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if(CurrentState == EGoblinZombieState::Dead)
		return;
	
	if(!PlayerCharacter)
		return;
	
	float distanceToPlayer = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());
	
	// Check Attack FIRST (smallest range)
	if(distanceToPlayer < AttackRange)
	{
		SetState(EGoblinZombieState::Attack);
	}
	// Chase when in detection range but NOT in attack range
	else if(distanceToPlayer < DetectionRange)
	{
		SetState(EGoblinZombieState::Chase);
	}
	// Patrol when out of detection range
	else
	{
		SetState(EGoblinZombieState::Patrol);
	}
	
	UpdateAIState();
}

void AGoblinZombieCharacter::SetState(EGoblinZombieState NewState)
{
    if (CurrentState == NewState)
        return;

    CurrentState = NewState;

    float NewSpeed = 0.0f;

    switch (CurrentState)
    {
        case EGoblinZombieState::Patrol:
            GetCharacterMovement()->bOrientRotationToMovement = true;  // Auto-rotate toward movement
            NewSpeed = PatrolSpeed;
            break;
        
        case EGoblinZombieState::Chase:
            GetCharacterMovement()->bOrientRotationToMovement = true;  // Auto-rotate toward movement (player)
            NewSpeed = ChaseSpeed;
            break;
        
        case EGoblinZombieState::Attack:
            GetCharacterMovement()->bOrientRotationToMovement = false;  // Disable auto-rotation
            NewSpeed = 0.0f;
            break;
        
        case EGoblinZombieState::Dead:
            GetCharacterMovement()->DisableMovement();
            break;
    }

    // Update animation ground speed
    if (GoblinZombieAnimInstance)
    {
        FFloatProperty* SpeedProperty = FindFProperty<FFloatProperty>(
            GoblinZombieAnimInstance->GetClass(),
            TEXT("Ground Speed")
        );

        if (SpeedProperty)
        {
            SpeedProperty->SetValue_InContainer(GoblinZombieAnimInstance, NewSpeed);
        }
    }

    GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void AGoblinZombieCharacter::UpdateAIState()
{
	switch(CurrentState)
	{
	case EGoblinZombieState::Patrol:
		Patrol();
		break;
	case EGoblinZombieState::Chase:
		ChasePlayer();
		break;
	case EGoblinZombieState::Attack:
		AttackPlayer();
		break;
	case EGoblinZombieState::Dead:
		// Do nothing
		break;
	default:
		break;
	}
}
void AGoblinZombieCharacter::Patrol()
{
	if(!GoblinController)
		return;
	FVector CurrentLocation = GetActorLocation();
	float distanceToTarget = FVector::Dist(CurrentLocation, CurrentPatrolTarget);
	if (distanceToTarget < 100.0f)
	{
		SelectRandomPatrolPoint();
	}
	GoblinController->MoveToLocation(CurrentPatrolTarget, 10.0f);
}

void AGoblinZombieCharacter::SelectRandomPatrolPoint()
{
	FVector RandomOffset = FVector(FMath::FRand() - 0.5f, FMath::FRand() - 0.5f, 0.0f).GetSafeNormal() * 1000.0f;
	CurrentPatrolTarget = GetActorLocation() + RandomOffset;
}
void AGoblinZombieCharacter::ChasePlayer()
{
	if(!GoblinController || !PlayerCharacter)
		return;
	GoblinController->MoveToActor(PlayerCharacter, AttackRange);
}
void AGoblinZombieCharacter::AttackPlayer()
{
    if (!PlayerCharacter)
        return;

    // Disable auto-rotation so we can manually rotate
    GetCharacterMovement()->bOrientRotationToMovement = false;

    // Face the player - rotate the capsule (actor)
    FVector DirectionToPlayer = (PlayerCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FRotator NewRotation = DirectionToPlayer.Rotation();
    
    // Rotate only yaw to face player
    FRotator YawOnlyRotation(0.0f, NewRotation.Yaw, 0.0f);
    SetActorRotation(YawOnlyRotation);
	
   
}

void AGoblinZombieCharacter::TakeDamage(float Damage)
{
	Health -= Damage;
	if(Health <= 0.0f)
	{
		Die();
	}
}
void AGoblinZombieCharacter::Die()
{
	SetState(EGoblinZombieState::Dead);
	// Play death animation
/*	if (ZombieAnimInstance)
	{
		ZombieAnimInstance->Montage_Play(DeathMontage);
	}*/
	// Disable collision and movement
//	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//	GetCharacterMovement()->DisableMovement();
	// Destroy the actor after a delay
//	SetLifeSpan(5.0f);
}













