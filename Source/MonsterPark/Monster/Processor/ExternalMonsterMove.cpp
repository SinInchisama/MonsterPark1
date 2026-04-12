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

			const float DetectRangeSq = FMath::Square(800.0f); // 탐지 거리
			const float LoseRangeSq = FMath::Square(1000.0f);   // 추적 포기 거리
			const float AttackRangeSq = FMath::Square(300.0f); // 공격 거리
			const float ArrivalThresholdSq = FMath::Square(500.0f);	// 배회 시 도착 판정 거리

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

				AActor* TargetHero = MoveData.TargetHero.Get();

				if (!TargetHero)
				{
					TargetHero = PlaySubsystem->FindNearestHeroInGrid(CurrentLocation, 500.0f); 
					if (TargetHero) MoveData.TargetHero = TargetHero;
				}

				if (TargetHero)
				{
					float DistSqToHero = FVector::DistSquared(CurrentLocation, TargetHero->GetActorLocation());

					if (DistSqToHero > LoseRangeSq)
					{
						MoveData.TargetHero = nullptr;
						TargetHero = nullptr;
					}
					else if (DistSqToHero <= AttackRangeSq)
					{
						bIsAttacking = true;

						if (MoveData.AttackCooldown <= 0.f)
						{
							if (ACharacterBase* Hero = Cast<ACharacterBase>(TargetHero))
							{
								Hero->TakeMonsterDamage(10.0f, CurrentLocation);
							}

							MoveData.AttackCooldown = 1.0f;
						}
					}
					else
					{
						MoveData.TargetLocation = TargetHero->GetActorLocation();
					}
				}

				if (!TargetHero)
				{
					if (MoveData.OriginLocation.IsZero()) MoveData.OriginLocation = CurrentLocation;

					float DistSqToTarget = FVector::DistSquared(CurrentLocation, MoveData.TargetLocation);
					if (MoveData.TargetLocation.IsZero() || DistSqToTarget < ArrivalThresholdSq)
					{
						FVector2D RandomOffset = FMath::RandPointInCircle(MoveData.WanderRadius);
						MoveData.TargetLocation = MoveData.OriginLocation + FVector(RandomOffset.X, RandomOffset.Y, 0.0f);
					}
				}

				FVector NextLocation = CurrentLocation;

				if (!bIsAttacking)
				{
					FVector Direction = (MoveData.TargetLocation - CurrentLocation).GetSafeNormal2D();
					float DistSqToTarget = FVector::DistSquared(CurrentLocation, MoveData.TargetLocation);

					if (!Direction.IsNearlyZero() && (TargetHero || DistSqToTarget > ArrivalThresholdSq))
					{
						NextLocation = FMath::VInterpConstantTo(CurrentLocation, MoveData.TargetLocation, DeltaTime, MoveData.Speed);
						NextLocation.Z = CurrentLocation.Z;
					}
				}

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
					FVector LookDirection = bIsAttacking ? (TargetHero->GetActorLocation() - CurrentLocation).GetSafeNormal2D()
						: (MoveData.TargetLocation - CurrentLocation).GetSafeNormal2D();

					if (!LookDirection.IsNearlyZero())
					{
						FVector OffsetForward(LookDirection.Y, -LookDirection.X, 0.0f);
						FVector TerrainNormal = HitResult.ImpactNormal;

						FQuat FinalQuat = FRotationMatrix::MakeFromXZ(OffsetForward, TerrainNormal).ToQuat();
						Transform.SetRotation(FinalQuat);
					}
				}
				Transform.SetLocation(NextLocation);

				FMassEntityHandle EntityHandle = Context.GetEntity(i);
				PlaySubsystem->UpdateMonsterLocation(EntityHandle, MoveData.LastGridKey, NextLocation);
			}
	});
}