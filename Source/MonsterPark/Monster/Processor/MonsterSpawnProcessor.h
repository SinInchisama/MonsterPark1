// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MonsterSpawnProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API UMonsterSpawnProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMonsterSpawnProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	

private:
	UPROPERTY()
	FMassEntityQuery EntityQuery;
};
