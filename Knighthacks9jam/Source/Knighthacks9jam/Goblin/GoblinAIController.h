// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GoblinAIController.generated.h"


UCLASS()
class KNIGHTHACKS9JAM_API AGoblinAIController : public AAIController
{
	GENERATED_BODY()
public:
	AGoblinAIController();
	virtual void BeginPlay() override;
	void MoveToLocation(FVector Location, float AcceptanceRadius);
	void MoveToActor(AActor* Target, float AcceptanceRadius);
};
