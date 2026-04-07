// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "FMonsterTargetFragment.generated.h"


/**
 * 
 */
USTRUCT()
struct MONSTERPARK_API FMonsterTargetFragment : public FMassFragment
{
    GENERATED_BODY()
    FVector Target;

    int TargetIndex = 0;

    float SpawnTime = -1.f;     // 추후 분리 예정
    bool bSpawned = false;

    bool Death = false;

};

static const FVector MonsterTargets[4] =
{
    FVector(-2600, 400, 60.f),
    FVector(-2600,  2600, 60.f),
    FVector(-400,  2600, 60.f),
    FVector(-400, 400, 60.f),
};