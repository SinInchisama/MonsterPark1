#pragma once


#include "ExternalMonsterMove.h" 

#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "MassCommonFragments.h"

#include "MonsterPark/Monster/Fragment/FMonsterRandomMoveFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MonsterPark/Monster/Tag/KilledTag.h"
#include "MonsterPark/Monster/Tag/MonsterDyingTag.h"
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

    FVector FuturePos = CurrentLocation + (DesiredDirection * LookAheadDistance);
    FIntVector TargetGrid = PlaySubsystem->PosToGrid(FuturePos);

    if (!PlaySubsystem->IsGridBlocked(TargetGrid))
    {
        return DesiredDirection;
    }

    for (float Angle = 30.f; Angle <= 150.f; Angle += 30.f)
    {
        FVector LeftDir = DesiredDirection.RotateAngleAxis(-Angle, FVector::UpVector);
        FVector RightDir = DesiredDirection.RotateAngleAxis(Angle, FVector::UpVector);

        if (!PlaySubsystem->IsGridBlocked(PlaySubsystem->PosToGrid(CurrentLocation + LeftDir * LookAheadDistance)))
            return LeftDir;

        if (!PlaySubsystem->IsGridBlocked(PlaySubsystem->PosToGrid(CurrentLocation + RightDir * LookAheadDistance)))
            return RightDir;
    }

    return DesiredDirection;
}

UExternalMonsterMove::UExternalMonsterMove() : EntityQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UExternalMonsterMove::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    EntityQuery.AddTagRequirement<FKilledTag>(EMassFragmentPresence::None);
    EntityQuery.AddTagRequirement<FMonsterDyingTag>(EMassFragmentPresence::None);


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

            const bool bIsFinalRound = PlaySubsystem->CurrentRound >= 20;

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

                const float MapMinBound = -12600.0f;
                const float MapMaxBound = 12600.0f;

                if (MoveData.AttackCooldown > 0.f) MoveData.AttackCooldown -= DeltaTime;
                AActor* CurrentTarget = MoveData.TargetHero.Get();

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

                if (!IsValid(CurrentTarget))
                {
                    if (MoveData.OriginLocation.IsZero()) MoveData.OriginLocation = CurrentLocation;
                    if (MoveData.TargetLocation.IsZero() ||
                        FVector::DistSquared2D(CurrentLocation, MoveData.TargetLocation) < ArrivalThresholdSq)
                    {
                        FVector2D RandomOffset = FMath::RandPointInCircle(MoveData.WanderRadius);
                        FVector NewTarget = MoveData.OriginLocation + FVector(RandomOffset.X, RandomOffset.Y, 0.0f);

                        NewTarget.X = FMath::Clamp(NewTarget.X, MapMinBound, MapMaxBound);
                        NewTarget.Y = FMath::Clamp(NewTarget.Y, MapMinBound, MapMaxBound);

                        MoveData.TargetLocation = NewTarget;
                    }
                }

                FVector NextLocation = CurrentLocation;
                if (!bIsAttacking)
                {
                    FVector Direction = (MoveData.TargetLocation - CurrentLocation).GetSafeNormal2D();
                    if (!Direction.IsNearlyZero())
                    {
                        FVector FlatCurrentLocation = FVector(CurrentLocation.X, CurrentLocation.Y, 0.f);
                        const float AdaptiveCheckDistance = 120.f;
                        FVector SteeredDir = GetGridSteeringDirection(PlaySubsystem, FlatCurrentLocation, Direction, AdaptiveCheckDistance);

                        float FinalSpeed = MoveData.Speed;
                        if (MoveData.SmoothedNormal.Z < 0.8f)
                        {
                            FinalSpeed *= 1.2f;
                        }

                        NextLocation = FMath::VInterpConstantTo(CurrentLocation, CurrentLocation + SteeredDir * 50.f, DeltaTime, FinalSpeed);
                    }
                }

 
                NextLocation.X = FMath::Clamp(NextLocation.X, MapMinBound, MapMaxBound);
                NextLocation.Y = FMath::Clamp(NextLocation.Y, MapMinBound, MapMaxBound);

                FHitResult HitResult;
                FVector StartTrace = NextLocation + FVector(0, 0, 1000.0f);
                FVector EndTrace = NextLocation - FVector(0, 0, 1000.0f);

                FIntVector CurrentGridKey = PlaySubsystem->PosToGrid(NextLocation);

                if (!PlaySubsystem->IsGridBlocked(CurrentGridKey))
                {
                    if (World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, QueryParams))
                    {
                        AActor* HitActor = HitResult.GetActor();
                        if (!(HitActor && HitActor->IsA(ACharacterBase::StaticClass())))
                        {
                            MoveData.CachedTerrainZ = HitResult.ImpactPoint.Z;

                            FVector LookTarget = bIsAttacking && IsValid(CurrentTarget) ?
                                (Cast<IHitInterface>(CurrentTarget) ? Cast<IHitInterface>(CurrentTarget)->GetTargetLocation(CurrentLocation) : CurrentTarget->GetActorLocation())
                                : MoveData.TargetLocation;

                            FVector LookDirection = (LookTarget - CurrentLocation).GetSafeNormal2D();

                            if (!LookDirection.IsNearlyZero())
                            {
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

                                FVector OffsetForward(LookDirection.Y, -LookDirection.X, 0.0f);
                                FQuat FinalQuat = FRotationMatrix::MakeFromXZ(OffsetForward, FinalNormal).ToQuat();
                                Transform.SetRotation(FinalQuat);
                            }
                        }
                    }
                }

                NextLocation.Z = MoveData.CachedTerrainZ;
                Transform.SetLocation(NextLocation);

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