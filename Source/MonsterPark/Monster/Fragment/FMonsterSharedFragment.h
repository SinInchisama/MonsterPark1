// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "FMonsterSharedFragment.generated.h"


/**
 *
 */
USTRUCT()
struct FMonsterRoundSharedFragment : public FMassSharedFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    float NextSpawnTime = 0.f; // 다음 스폰까지 남은 시간
};