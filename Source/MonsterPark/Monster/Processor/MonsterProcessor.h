// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MonsterProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API UMonsterProcessor : public UMassProcessor
{
	GENERATED_BODY()
public:
    UMonsterProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    UPROPERTY()
    FMassEntityQuery EntityQuery;

    int32 SpawnIndex = 0;
};
