// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MonsterHitVFXProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API UMonsterHitVFXProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UMonsterHitVFXProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery HitQuery;

};
