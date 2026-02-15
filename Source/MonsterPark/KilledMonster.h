// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassObserverProcessor.h"
#include "KilledMonster.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API UKilledMonster : public UMassObserverProcessor
{
    GENERATED_BODY()
public:
    UKilledMonster();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    UPROPERTY()
    FMassEntityQuery EntityQuery;
	
};
