#pragma once

// 1. 무조건 해당 클래스의 헤더가 최상단!
#include "ExternalMonsterMove.h" 

// 2. 프로젝트 및 Mass Entity 필수 헤더
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "MassCommonFragments.h"

// 3. 사용자가 정의한 프래그먼트 및 기타 엔진 헤더
#include "MonsterPark/Monster/Fragment/FMonsterRandomMoveFragment.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

#include "MonsterPark/PlaySubSystem.h"
#include "MonsterPark/CharacterBase.h"

UExternalMonsterMove::UExternalMonsterMove() : EntityQuery(*this)
{
	// DamageCheckProcessor와 동일하게 페이즈와 그룹 설정
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UExternalMonsterMove::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// 정상 작동 코드(DamageCheck)의 방식을 그대로 따름
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMonsterRandomMoveFragment>(EMassFragmentAccess::ReadWrite);
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

            // 설정값
            const float LoseRangeSq = FMath::Square(1000.0f);
            const float AttackRangeSq = FMath::Square(300.0f);
            const float ArrivalThresholdSq = FMath::Square(500.0f);
            const float DetectRange = 800.0f;

            const bool bIsFinalRound = PlaySubsystem->CurrentRound >= 0; // 로직에 따라 수정 가능

            FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MassMonsterTrace), false);

            const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
            const TArrayView<FMonsterRandomMoveFragment> MoveList = Context.GetMutableFragmentView<FMonsterRandomMoveFragment>();

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {
                FTransform& Transform = TransformList[i].GetMutableTransform();
                FMonsterRandomMoveFragment& MoveData = MoveList[i];
                FVector CurrentLocation = Transform.GetLocation();
                bool bIsAttacking = false;

                if (MoveData.AttackCooldown > 0.f) MoveData.AttackCooldown -= DeltaTime;
                AActor* CurrentTarget = MoveData.TargetHero.Get();

                if (bIsFinalRound)
                {
                    if (!IsValid(CurrentTarget) || CurrentTarget->IsA(ACharacterBase::StaticClass()))
                    {
                        if (MoveData.bHasFinishedWall)
                        {
                            CurrentTarget = PlaySubsystem->Nexus;
                        }
                        else
                        {
                            CurrentTarget = PlaySubsystem->FindFinalRoundTarget(CurrentLocation);

                            if (CurrentTarget)
                            {

                                MoveData.bHasFinishedWall = true;
                            }
                            else
                            {
                                CurrentTarget = PlaySubsystem->Nexus;
                            }
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

                // 3. 타겟 상호작용 (위치 가져오기 및 공격)
                if (IsValid(CurrentTarget))
                {
                    // [중요] 인터페이스를 통해 실제 '메시' 혹은 '타겟' 위치를 가져옴
                    IHitInterface* HitInterface = Cast<IHitInterface>(CurrentTarget);
                    FVector TargetPos = HitInterface ? HitInterface->GetTargetLocation(CurrentLocation) : CurrentTarget->GetActorLocation();

                    float DistSqToTarget = FVector::DistSquared(CurrentLocation, TargetPos);

                    // 유효 거리 체크 (추적 포기 - 일반 라운드 전용)
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
                            // [핵심] 물리/액터 수정을 포함할 수 있으므로 GameThread에서 실행
                            AsyncTask(ENamedThreads::GameThread, [HitInterface, CurrentLocation]()
                                {
                                    if (HitInterface)
                                    {
                                        HitInterface->TakeMonsterDamage(10.0f, CurrentLocation);
                                    }
                                });
                            MoveData.AttackCooldown = 1.0f;
                        }
                    }
                    else
                    {
                        MoveData.TargetLocation = TargetPos;
                    }
                }

                // 4. 배회 로직 (타겟이 없을 때)
                if (!IsValid(CurrentTarget))
                {
                    if (MoveData.OriginLocation.IsZero()) MoveData.OriginLocation = CurrentLocation;

                    float DistSqToWanderTarget = FVector::DistSquared(CurrentLocation, MoveData.TargetLocation);
                    if (MoveData.TargetLocation.IsZero() || DistSqToWanderTarget < ArrivalThresholdSq)
                    {
                        FVector2D RandomOffset = FMath::RandPointInCircle(MoveData.WanderRadius);
                        MoveData.TargetLocation = MoveData.OriginLocation + FVector(RandomOffset.X, RandomOffset.Y, 0.0f);
                    }
                }

                // 5. 이동 계산
                FVector NextLocation = CurrentLocation;
                if (!bIsAttacking)
                {
                    FVector Direction = (MoveData.TargetLocation - CurrentLocation).GetSafeNormal2D();
                    if (!Direction.IsNearlyZero())
                    {
                        NextLocation = FMath::VInterpConstantTo(CurrentLocation, MoveData.TargetLocation, DeltaTime, MoveData.Speed);
                    }
                }

                // 6. 지형 스내핑 및 회전
                FHitResult HitResult;
                FVector StartTrace = NextLocation + FVector(0, 0, 1000.0f);
                FVector EndTrace = NextLocation - FVector(0, 0, 1000.0f);

                if (World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, QueryParams))
                {
                    AActor* HitActor = HitResult.GetActor();
                    // 영웅 밟기 방지
                    if (!(HitActor && HitActor->IsA(ACharacterBase::StaticClass())))
                    {
                        NextLocation.Z = HitResult.ImpactPoint.Z;
                    }

                    // 회전 계산용 타겟 위치 (공격 중이면 타겟 메시, 아니면 이동 지점)
                    FVector LookTarget = bIsAttacking && IsValid(CurrentTarget) ?
                        (Cast<IHitInterface>(CurrentTarget) ? Cast<IHitInterface>(CurrentTarget)->GetTargetLocation(CurrentLocation) : CurrentTarget->GetActorLocation())
                        : MoveData.TargetLocation;

                    FVector LookDirection = (LookTarget - CurrentLocation).GetSafeNormal2D();

                    if (!LookDirection.IsNearlyZero())
                    {
                        FVector OffsetForward(LookDirection.Y, -LookDirection.X, 0.0f);
                        FVector TerrainNormal = HitResult.ImpactNormal;
                        FQuat FinalQuat = FRotationMatrix::MakeFromXZ(OffsetForward, TerrainNormal).ToQuat();
                        Transform.SetRotation(FinalQuat);
                    }
                }

                // 7. 최종 위치 적용 및 그리드 업데이트
                Transform.SetLocation(NextLocation);
                FMassEntityHandle EntityHandle = Context.GetEntity(i);
                PlaySubsystem->UpdateMonsterLocation(EntityHandle, MoveData.LastGridKey, NextLocation);
            }
        });
}