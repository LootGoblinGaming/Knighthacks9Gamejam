// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieCharacter.generated.h"
enum class EZombieState : uint8
{
	Patrol,
	Chase,
	Attack,
	Dead
};

UCLASS()
class KNIGHTHACKS9JAM_API AZombieCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AZombieCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// state management
	void setState(EZombieState newState);
	void UpdateState();
	void patrol();
	void chase();
	void attack();

	// TakeDamage
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
