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
	// DamageCheckProcessor처럼 Context만 전달하는 ForEachEntityChunk 사용
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
			const TArrayView<FMonsterRandomMoveFragment> MoveList = Context.GetMutableFragmentView<FMonsterRandomMoveFragment>();

			const float DeltaTime = Context.GetDeltaTimeSeconds();
			UWorld* World = Context.GetWorld();

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {
                FTransform& Transform = TransformList[i].GetMutableTransform();
                FMonsterRandomMoveFragment& MoveData = MoveList[i];
                FVector CurrentLocation = Transform.GetLocation();

                // [1] 기준점 초기화
                if (MoveData.OriginLocation.IsZero())
                {
                    MoveData.OriginLocation = CurrentLocation;
                }

                // [2] 도착 판정 및 목표 갱신
                float DistSqToTarget = FVector::DistSquared(CurrentLocation, MoveData.TargetLocation);
                const float ArrivalThreshold = 500.0f; // 너무 크면 목적지 근처에서 회전이 어색하므로 100 정도로 조절 권장
                const float ArrivalThresholdSq = ArrivalThreshold * ArrivalThreshold;

                if (MoveData.TargetLocation.IsZero() || DistSqToTarget < ArrivalThresholdSq)
                {
                    FVector2D RandomOffset = FMath::RandPointInCircle(MoveData.WanderRadius);
                    MoveData.TargetLocation = MoveData.OriginLocation + FVector(RandomOffset.X, RandomOffset.Y, 0.0f);
                    DistSqToTarget = FVector::DistSquared(CurrentLocation, MoveData.TargetLocation);
                }

                // [3] 기본 이동 방향 (회전의 기본값이 됩니다)
                FVector NextLocation = CurrentLocation;
                FVector Direction = (MoveData.TargetLocation - CurrentLocation).GetSafeNormal2D();

                // 이동 계산
                if (!Direction.IsNearlyZero() && DistSqToTarget > ArrivalThresholdSq)
                {
                    FVector MoveStep = Direction * MoveData.Speed * DeltaTime;
                    if (MoveStep.SizeSquared() > DistSqToTarget)
                    {
                        NextLocation.X = MoveData.TargetLocation.X;
                        NextLocation.Y = MoveData.TargetLocation.Y;
                    }
                    else
                    {
                        NextLocation += MoveStep;
                    }
                }

                // [4] 지형 인식 및 몸 기울기 계산
                if (World)
                {
                    FHitResult HitResult;
                    FVector StartTrace = NextLocation + FVector(0, 0, 1000.0f);
                    FVector EndTrace = NextLocation - FVector(0, 0, 1000.0f);

                    FCollisionQueryParams QueryParams;

                    if (World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, QueryParams))
                    {
                        // 바닥 높이 적용
                        AActor* HitActor = HitResult.GetActor();
                        if (HitActor && HitActor->IsA(ACharacterBase::StaticClass()))
                        {
                            // 캐릭터를 맞췄다면 위치를 업데이트하지 않거나, 
                            // 다시 레이를 쏴서 바닥을 찾게 해야 합니다. (복잡해짐)
                        }
                        else
                            NextLocation.Z = HitResult.ImpactPoint.Z;

                        if (!Direction.IsNearlyZero())
                        {
                            // 1. 사용자님이 검증한 "정확히 바라보는 방향"의 Yaw값 추출
                            FRotator CorrectYawRot = Direction.Rotation();
                            CorrectYawRot.Pitch = 0.f;
                            CorrectYawRot.Roll = 0.f;

                            // 2. [수정 핵심] 몬스터가 옆을 보고 있다면? Yaw에 90도를 더해줍니다.
                            // 만약 반대로 돌았다면 -90을 해주세요.
                            CorrectYawRot.Yaw -= 90.0f;

                            // 3. 지형 기울기(Normal) 가져오기
                            FVector TerrainNormal = HitResult.ImpactNormal;

                            // 4. 오프셋이 적용된 Forward와 Normal을 조합
                            FVector OffsetForward = CorrectYawRot.Vector();
                            FQuat FinalQuat = FRotationMatrix::MakeFromXZ(OffsetForward, TerrainNormal).ToQuat();

                            // 5. 최종 회전 적용
                            Transform.SetRotation(FinalQuat);
                        }
                    }
                }

                // [5] 최종 위치 적용
                Transform.SetLocation(NextLocation);
            }
		});
}