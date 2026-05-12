#pragma once


#include "ExternalMonsterMove.h" 

#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "MassCommonFragments.h"

#include "MonsterPark/Monster/Fragment/FMonsterRandomMoveFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

#include "MonsterPark/PlaySubSystem.h"
#include "MonsterPark/CharacterBase.h"

#include "MassRepresentationFragments.h"
#include "MassActorSubsystem.h"

#include"MonsterPark/Monster/AttackVisualActor.h"

static FVector GetGridSteeringDirection(
    const UPlaySubSystem* PlaySubsystem,
    const FVector& CurrentLocation,
    const FVector& DesiredDirection,
    float LookAheadDistance)
{
    if (!PlaySubsystem) return DesiredDirection;

    // 1. 정면 격자 확인
    FVector FuturePos = CurrentLocation + (DesiredDirection * LookAheadDistance);
    FIntVector TargetGrid = PlaySubsystem->PosToGrid(FuturePos);

    // 정면이 뚫려있으면 그대로 진행
    if (!PlaySubsystem->IsGridBlocked(TargetGrid))
    {
        return DesiredDirection;
    }

    // 2. 정면이 막혔다면 좌우로 각도를 벌리며 비어있는 격자 탐색 (30도씩 최대 150도까지)
    for (float Angle = 30.f; Angle <= 150.f; Angle += 30.f)
    {
        FVector LeftDir = DesiredDirection.RotateAngleAxis(-Angle, FVector::UpVector);
        FVector RightDir = DesiredDirection.RotateAngleAxis(Angle, FVector::UpVector);

        if (!PlaySubsystem->IsGridBlocked(PlaySubsystem->PosToGrid(CurrentLocation + LeftDir * LookAheadDistance)))
            return LeftDir;

        if (!PlaySubsystem->IsGridBlocked(PlaySubsystem->PosToGrid(CurrentLocation + RightDir * LookAheadDistance)))
            return RightDir;
    }

    // 모든 방향이 막혔다면 일단 원래 방향 유지 (벽에 비빔)
    return DesiredDirection;
}

UExternalMonsterMove::UExternalMonsterMove() : EntityQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UExternalMonsterMove::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMonsterRandomMoveFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMonsterStatusFragment>(EMassFragmentAccess::ReadWrite);

    EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite);
}

void UExternalMonsterMove::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
        {
            UWorld* World = Context.GetWorld();
            if (!World) return;

            UPlaySubSystem* PlaySubsystem = World->GetSubsystem<UPlaySubSystem>();
            if (!PlaySubsystem) return;

            const float DeltaTime = Context.GetDeltaTimeSeconds();
            const float LoseRangeSq = FMath::Square(1000.0f);
            const float AttackRangeSq = FMath::Square(300.0f);
            const float ArrivalThresholdSq = FMath::Square(500.0f);
            const float DetectRange = 800.0f;
            const float GridCheckDistance = 150.f;

            const bool bIsFinalRound = PlaySubsystem->CurrentRound >= 5;

            FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MassMonsterTrace), false);

            auto TransformList = Context.GetMutableFragmentView<FTransformFragment>();
            auto MoveList = Context.GetMutableFragmentView<FMonsterRandomMoveFragment>();
            auto ActorList = Context.GetMutableFragmentView<FMassActorFragment>();
            auto StatusList = Context.GetMutableFragmentView<FMonsterStatusFragment>();

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {
                FTransform& Transform = TransformList[i].GetMutableTransform();
                FMonsterRandomMoveFragment& MoveData = MoveList[i];
                FVector CurrentLocation = Transform.GetLocation();
                bool bIsAttacking = false;

                if (MoveData.AttackCooldown > 0.f) MoveData.AttackCooldown -= DeltaTime;
                AActor* CurrentTarget = MoveData.TargetHero.Get();

                // --- 타겟팅 로직 (기존 유지) ---
                if (bIsFinalRound)
                {
                    if (!IsValid(CurrentTarget) || CurrentTarget->IsA(ACharacterBase::StaticClass()))
                    {
                        if (MoveData.bHasFinishedWall) CurrentTarget = PlaySubsystem->Nexus;
                        else
                        {
                            CurrentTarget = PlaySubsystem->FindFinalRoundTarget(CurrentLocation);
                            if (CurrentTarget) MoveData.bHasFinishedWall = true;
                            else CurrentTarget = PlaySubsystem->Nexus;
                        }
                        MoveData.TargetHero = CurrentTarget;
                    }
                }
                else
                {
                    if (!IsValid(CurrentTarget))
                    {
                        CurrentTarget = PlaySubsystem->FindNearestHeroInGrid(CurrentLocation, DetectRange);
                        if (CurrentTarget) MoveData.TargetHero = CurrentTarget;
                    }
                }

                // --- 타겟 위치 및 공격 판정 ---
                if (IsValid(CurrentTarget))
                {
                    IHitInterface* HitInterface = Cast<IHitInterface>(CurrentTarget);
                    FVector TargetPos = HitInterface ? HitInterface->GetTargetLocation(CurrentLocation) : CurrentTarget->GetActorLocation();

                    float DistSqToTarget = FVector::DistSquared(CurrentLocation, TargetPos);
                    if (!bIsFinalRound && DistSqToTarget > LoseRangeSq)
                    {
                        MoveData.TargetHero = nullptr;
                        CurrentTarget = nullptr;
                    }
                    else if (DistSqToTarget <= AttackRangeSq)
                    {
                        bIsAttacking = true;
                        if (MoveData.AttackCooldown <= 0.f && HitInterface)
                        {
                            AActor* VisualActor = ActorList[i].GetMutable();
                            AsyncTask(ENamedThreads::GameThread, [HitInterface, CurrentLocation, VisualActor]()
                                {
                                    if (AAttackVisualActor* AttackActor = Cast<AAttackVisualActor>(VisualActor))
                                        AttackActor->PlayAttackAnimation();
                                    if (HitInterface)
                                        HitInterface->TakeMonsterDamage(10.0f, CurrentLocation);
                                });
                            MoveData.AttackCooldown = 1.0f;
                        }
                    }
                    else MoveData.TargetLocation = TargetPos;
                }

                // --- Wander 로직 ---
                if (!IsValid(CurrentTarget))
                {
                    if (MoveData.OriginLocation.IsZero()) MoveData.OriginLocation = CurrentLocation;
                    if (MoveData.TargetLocation.IsZero() || FVector::DistSquared(CurrentLocation, MoveData.TargetLocation) < ArrivalThresholdSq)
                    {
                        FVector2D RandomOffset = FMath::RandPointInCircle(MoveData.WanderRadius);
                        MoveData.TargetLocation = MoveData.OriginLocation + FVector(RandomOffset.X, RandomOffset.Y, 0.0f);
                    }
                }

                // --- 이동 계산 (격자 회피 적용) ---
                FVector NextLocation = CurrentLocation;
                if (!bIsAttacking)
                {
                    FVector Direction = (MoveData.TargetLocation - CurrentLocation).GetSafeNormal2D();
                    if (!Direction.IsNearlyZero())
                    {
                        // 격자 기반 방향 수정
                        FVector SteeredDir = GetGridSteeringDirection(PlaySubsystem, CurrentLocation, Direction, GridCheckDistance);
                        NextLocation = FMath::VInterpConstantTo(CurrentLocation, CurrentLocation + SteeredDir * 100.f, DeltaTime, MoveData.Speed);
                    }
                }

                // --- 지형 트레이스 및 회전 (원래 코드 로직 복구) ---
                FHitResult HitResult;
                FVector StartTrace = NextLocation + FVector(0, 0, 1000.0f);
                FVector EndTrace = NextLocation - FVector(0, 0, 1000.0f);

                FIntVector CurrentGridKey = PlaySubsystem->PosToGrid(NextLocation);

                // 격자가 막히지 않았을 때만 지형 높이 갱신
                if (!PlaySubsystem->IsGridBlocked(CurrentGridKey))
                {
                    if (World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, QueryParams))
                    {
                        AActor* HitActor = HitResult.GetActor();
                        if (!(HitActor && HitActor->IsA(ACharacterBase::StaticClass())))
                        {
                            MoveData.CachedTerrainZ = HitResult.ImpactPoint.Z;

                            // 원래의 회전 로직 시작
                            FVector LookTarget = bIsAttacking && IsValid(CurrentTarget) ?
                                (Cast<IHitInterface>(CurrentTarget) ? Cast<IHitInterface>(CurrentTarget)->GetTargetLocation(CurrentLocation) : CurrentTarget->GetActorLocation())
                                : MoveData.TargetLocation;

                            FVector LookDirection = (LookTarget - CurrentLocation).GetSafeNormal2D();

                            if (!LookDirection.IsNearlyZero())
                            {
                                // Normal 스무딩
                                MoveData.SmoothedNormal = FMath::VInterpTo(MoveData.SmoothedNormal, HitResult.ImpactNormal, DeltaTime, 5.0f);

                                FVector UpVector = FVector::UpVector;
                                float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(MoveData.SmoothedNormal, UpVector)));
                                float MaxTiltAngle = 30.0f;

                                FVector FinalNormal = MoveData.SmoothedNormal;
                                if (AngleDeg > MaxTiltAngle)
                                {
                                    FVector RotationAxis = FVector::CrossProduct(UpVector, MoveData.SmoothedNormal).GetSafeNormal();
                                    if (!RotationAxis.IsNearlyZero())
                                    {
                                        FQuat LimitQuat = FQuat(RotationAxis, FMath::DegreesToRadians(MaxTiltAngle));
                                        FinalNormal = LimitQuat.RotateVector(UpVector);
                                    }
                                }

                                // 원래 쓰시던 방향 계산 방식: LookDirection의 Y, -X를 사용하는 축 정렬
                                FVector OffsetForward(LookDirection.Y, -LookDirection.X, 0.0f);
                                FQuat FinalQuat = FRotationMatrix::MakeFromXZ(OffsetForward, FinalNormal).ToQuat();
                                Transform.SetRotation(FinalQuat);
                            }
                        }
                    }
                }

                // 최종 위치 적용 (Z는 항상 캐시된 지형 높이 사용)
                NextLocation.Z = MoveData.CachedTerrainZ;
                Transform.SetLocation(NextLocation);

                // 데이터 동기화
                FMassEntityHandle EntityHandle = Context.GetEntity(i);
                PlaySubsystem->UpdateMonsterLocation(EntityHandle, MoveData.LastGridKey, NextLocation);

                if (ActorList[i].IsValid())
                {
                    if (AActor* VisualActor = ActorList[i].GetMutable())
                        VisualActor->SetActorTransform(Transform);
                }
            }
        });
}