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

			// 1. Subsystem 가져오기 (루프 밖에서 한 번만 캐싱)
			UPlaySubSystem* PlaySubsystem = World->GetSubsystem<UPlaySubSystem>();
			if (!PlaySubsystem) return;

			const float DeltaTime = Context.GetDeltaTimeSeconds();

			// 엔티티마다 변하지 않는 상수 및 설정은 루프 밖으로 이동
			const float ArrivalThresholdSq = FMath::Square(500.0f);
			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MassMonsterTrace), false);

			const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
			const TArrayView<FMonsterRandomMoveFragment> MoveList = Context.GetMutableFragmentView<FMonsterRandomMoveFragment>();

			for (int32 i = 0; i < Context.GetNumEntities(); ++i)
			{
				FTransform& Transform = TransformList[i].GetMutableTransform();
				FMonsterRandomMoveFragment& MoveData = MoveList[i];
				FVector CurrentLocation = Transform.GetLocation();

				// 1. 초기 위치 설정
				if (MoveData.OriginLocation.IsZero())
				{
					MoveData.OriginLocation = CurrentLocation;
				}

				float DistSqToTarget = FVector::DistSquared(CurrentLocation, MoveData.TargetLocation);

				// 2. 새 목표 지점 설정 (도착했거나 타겟이 없는 경우)
				if (MoveData.TargetLocation.IsZero() || DistSqToTarget < ArrivalThresholdSq)
				{
					FVector2D RandomOffset = FMath::RandPointInCircle(MoveData.WanderRadius);
					MoveData.TargetLocation = MoveData.OriginLocation + FVector(RandomOffset.X, RandomOffset.Y, 0.0f);
					DistSqToTarget = FVector::DistSquared(CurrentLocation, MoveData.TargetLocation);
				}

				FVector NextLocation = CurrentLocation;
				FVector Direction = (MoveData.TargetLocation - CurrentLocation).GetSafeNormal2D();

				// 3. 이동 처리 (VInterpConstantTo를 사용해 오버슛 방지 및 간소화)
				if (!Direction.IsNearlyZero() && DistSqToTarget > ArrivalThresholdSq)
				{
					NextLocation = FMath::VInterpConstantTo(CurrentLocation, MoveData.TargetLocation, DeltaTime, MoveData.Speed);
					// Z축은 LineTrace로 결정할 것이므로, XY축만 갱신
					NextLocation.Z = CurrentLocation.Z;
				}

				// 4. 지형 스내핑 및 회전 (Line Trace)
				FHitResult HitResult;
				FVector StartTrace = NextLocation + FVector(0, 0, 1000.0f);
				FVector EndTrace = NextLocation - FVector(0, 0, 1000.0f);

				if (World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, QueryParams))
				{
					AActor* HitActor = HitResult.GetActor();
					if (!(HitActor && HitActor->IsA(ACharacterBase::StaticClass())))
					{
						NextLocation.Z = HitResult.ImpactPoint.Z;
					}

					// 5. 지형 경사면에 맞춘 회전 처리 간소화
					if (!Direction.IsNearlyZero())
					{
						// FRotator 변환 후 Yaw - 90을 하는 대신, 2D 벡터 회전 공식 적용
						// (X, Y)를 -90도 회전하면 (Y, -X)가 됨
						FVector OffsetForward(Direction.Y, -Direction.X, 0.0f);
						FVector TerrainNormal = HitResult.ImpactNormal;

						FQuat FinalQuat = FRotationMatrix::MakeFromXZ(OffsetForward, TerrainNormal).ToQuat();
						Transform.SetRotation(FinalQuat);
					}
				}

				// 최종 위치 적용
				Transform.SetLocation(NextLocation);

				FMassEntityHandle EntityHandle = Context.GetEntity(i);
				PlaySubsystem->UpdateMonsterLocation(EntityHandle, MoveData.LastGridKey, NextLocation);
			}
		});
}