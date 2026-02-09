// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityQuery.h"
#include "AMyDetectionActor.generated.h"

UCLASS()
class MONSTERPARK_API AAMyDetectionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	TArray<FMassEntityHandle> DetectedEnemies;
	FMassEntityQuery EnemyQuery;
	// Sets default values for this actor's properties
	AAMyDetectionActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void FindEnemiesInArea();

};
