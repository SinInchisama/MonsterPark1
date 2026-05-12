// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "BlockingObstacle.generated.h"

UCLASS()
class MONSTERPARK_API ABlockingObstacle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABlockingObstacle();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Obstacle")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Obstacle")
	UBoxComponent* GridCollisionBox;

	TArray<FIntVector> GetAffectedGridKeys() const;
public:	
	TArray<FIntVector> RegisteredKeys;
};
