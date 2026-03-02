// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSpawner.h"
#include "MyMassSpawner.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API AMyMassSpawner : public AMassSpawner
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Mass|Spawn")
	void SpawnEntityByIndex(int32 Index, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Mass|Spawn")
	int32 GetAliveCount() const;
};
