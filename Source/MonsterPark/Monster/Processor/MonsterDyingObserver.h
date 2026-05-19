// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MonsterDyingObserver.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API UMonsterDyingObserver : public UMassObserverProcessor
{
	GENERATED_BODY()
public:
	UMonsterDyingObserver();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
