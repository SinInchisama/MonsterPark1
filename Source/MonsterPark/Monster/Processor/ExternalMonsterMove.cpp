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

      
                if (MoveData.OriginLocation.IsZero())
                {
                    MoveData.OriginLocation = CurrentLocation;
                }
                float DistSqToTarget = FVector::DistSquared(CurrentLocation, MoveData.TargetLocation);
                const float ArrivalThreshold = 500.0f;
                const float ArrivalThresholdSq = ArrivalThreshold * ArrivalThreshold;

                if (MoveData.TargetLocation.IsZero() || DistSqToTarget < ArrivalThresholdSq)
                {
                    FVector2D RandomOffset = FMath::RandPointInCircle(MoveData.WanderRadius);
                    MoveData.TargetLocation = MoveData.OriginLocation + FVector(RandomOffset.X, RandomOffset.Y, 0.0f);
                    DistSqToTarget = FVector::DistSquared(CurrentLocation, MoveData.TargetLocation);
                }

  
                FVector NextLocation = CurrentLocation;
                FVector Direction = (MoveData.TargetLocation - CurrentLocation).GetSafeNormal2D();

      
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

    
                if (World)
                {
                    FHitResult HitResult;
                    FVector StartTrace = NextLocation + FVector(0, 0, 1000.0f);
                    FVector EndTrace = NextLocation - FVector(0, 0, 1000.0f);

                    FCollisionQueryParams QueryParams;

                    if (World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, QueryParams))
                    {

                        AActor* HitActor = HitResult.GetActor();
                        if (HitActor && HitActor->IsA(ACharacterBase::StaticClass()))
                        {
 
                        }
                        else
                            NextLocation.Z = HitResult.ImpactPoint.Z;

                        if (!Direction.IsNearlyZero())
                        {
 
                            FRotator CorrectYawRot = Direction.Rotation();
                            CorrectYawRot.Pitch = 0.f;
                            CorrectYawRot.Roll = 0.f;


                            CorrectYawRot.Yaw -= 90.0f;


                            FVector TerrainNormal = HitResult.ImpactNormal;

  
                            FVector OffsetForward = CorrectYawRot.Vector();
                            FQuat FinalQuat = FRotationMatrix::MakeFromXZ(OffsetForward, TerrainNormal).ToQuat();


                            Transform.SetRotation(FinalQuat);
                        }
                    }
                }


                Transform.SetLocation(NextLocation);
            }
		});
}