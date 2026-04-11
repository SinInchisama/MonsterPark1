// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChestSpawner.generated.h"

class ATreasureChest;

UCLASS()
class MONSTERPARK_API AChestSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AChestSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<ATreasureChest> ChestClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 ChestCount;

public:
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SpawnChests();
};
