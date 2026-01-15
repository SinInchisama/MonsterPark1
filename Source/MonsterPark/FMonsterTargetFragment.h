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

    float SpawnTime = -1.f;
    bool bSpawned = false;
};
