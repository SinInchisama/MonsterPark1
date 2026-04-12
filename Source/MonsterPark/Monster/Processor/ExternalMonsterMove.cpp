#pragma once


#include "ExternalMonsterMove.h" 

#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "MassCommonFragments.h"

#include "MonsterPark/Monster/Fragment/FMonsterRandomMoveFragment.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

#include "MonsterPark/PlaySubSystem.h"
#include "MonsterPark/CharacterBase.h"

UExternalMonsterMove::UExternalMonsterMove() : EntityQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UExternalMonsterMove::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
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

            const bool bIsFinalRound = PlaySubsystem->CurrentRound >= 5; // 로직에 따라 수정 가능

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

                FVector NextLocation = CurrentLocation;
                if (!bIsAttacking)
                {
                    FVector Direction = (MoveData.TargetLocation - CurrentLocation).GetSafeNormal2D();
                    if (!Direction.IsNearlyZero())
                    {
                        NextLocation = FMath::VInterpConstantTo(CurrentLocation, MoveData.TargetLocation, DeltaTime, MoveData.Speed);
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