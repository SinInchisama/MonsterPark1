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
    // 생성자 선언
    UMonsterProcessor();

protected:
    // 5.6에서 final이 아닌 인자 있는 버전을 유지합니다.
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
    int32 SpawnIndex = 0;
};
