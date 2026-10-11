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
	ZombieAnimInstance = GetMesh()->GetAnimInstance();
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
	if(distanceToPlayer < DetectionRange && CurrentState != EGoblinZombieState::Attack)
	{
		SetState(EGoblinZombieState::Chase);
	}
	else if(distanceToPlayer >= DetectionRange && CurrentState == EGoblinZombieState::Chase)
	{
		SetState(EGoblinZombieState::Patrol);
	}
	else if(distanceToPlayer < AttackRange)
	{
		SetState(EGoblinZombieState::Attack);
	}
	UpdateAIState();
}

void AGoblinZombieCharacter::SetState(EGoblinZombieState NewState)
{
	if(CurrentState == NewState)
		return;
	CurrentState = NewState;
	switch(CurrentState)
	{
	case EGoblinZombieState::Patrol:
		GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
		break;
	case EGoblinZombieState::Chase:
		GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
		break;
	case EGoblinZombieState::Attack:
		GetCharacterMovement()->MaxWalkSpeed = 0.0f;
		break;
	case EGoblinZombieState::Dead:
		GetCharacterMovement()->DisableMovement();
		break;
	default:
		break;
	}
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
	if(!PlayerCharacter)
		return;
	// Play attack animation
	// Simple attack - just face the player
	FVector DirectionToPlayer = (PlayerCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FRotator NewRotation = DirectionToPlayer.Rotation();
	SetActorRotation(NewRotation);
	// Deal damage to player (this is just a placeholder, you would need to implement actual damage logic)
	// PlayerCharacter->TakeDamage(10.0f, FDamageEvent(), GoblinController, this);
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

