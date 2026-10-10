// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAIController.h"
#include "ZombieCharacter.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

AZombieAIController::AZombieAIController()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AZombieAIController::BeginPlay()
{
    Super::BeginPlay();
}

void AZombieAIController::MoveToLocation(FVector Location, float AcceptanceRadius)
{
    UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Location);
}

void AZombieAIController::MoveToActor(AActor* Target, float AcceptanceRadius)
{
    if (Target)
    {
        UAIBlueprintHelperLibrary::SimpleMoveToActor(this, Target);
    }
}

void AZombieAIController::StopMovement()
{
    StopMovement();
}

