// Fill out your copyright notice in the Description page of Project Settings.


#include "GoblinAIController.h"
#include "GoblinZombieCharacter.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
AGoblinAIController::AGoblinAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGoblinAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AGoblinAIController::MoveToLocation(FVector Location, float AcceptanceRadius)
{
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Location);
}
void AGoblinAIController::MoveToActor(AActor* Target, float AcceptanceRadius)
{
	if (Target)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToActor(this, Target);
	}
}
