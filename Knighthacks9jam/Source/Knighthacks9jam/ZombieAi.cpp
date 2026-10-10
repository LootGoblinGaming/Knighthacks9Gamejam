// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAi.h"

// Sets default values
AZombieAi::AZombieAi()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AZombieAi::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AZombieAi::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AZombieAi::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

