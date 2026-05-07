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

    // 격자에 사용하는 키
    int64 LastGridKey = -1;

    // 현재 타켓팅 중인 영웅
    TWeakObjectPtr<AActor> TargetHero = nullptr;

    // 공격 쿨타임 타이머
    float AttackCooldown = 0.0f;

    // 성벽을 부셔는지 체크
    bool bHasFinishedWall = false;

    // 경사 올라갈 때 각도 보간
    FVector SmoothedNormal = FVector::UpVector;
};