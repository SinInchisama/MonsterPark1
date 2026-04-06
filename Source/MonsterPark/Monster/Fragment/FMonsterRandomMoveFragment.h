// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "FMonsterRandomMoveFragment.generated.h"

/**
 * 
 */
USTRUCT()
struct FMonsterRandomMoveFragment : public FMassFragment
{
    GENERATED_BODY()

    // 현재 이동하려는 목표 위치
    FVector TargetLocation = FVector::ZeroVector;

    // 배회를 시작한 중심점
    FVector OriginLocation = FVector::ZeroVector;

    // 이동 속도
    float Speed = 100.0f;

    // 배회할 최대 반경
    float WanderRadius = 1000.0f;
};