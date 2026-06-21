// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterProcessor.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MonsterPark/Monster/Fragment/FMonsterTargetFragment.h"
#include "MonsterPark//Monster/Fragment/FMonsterStatusFragment.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassMovementFragments.h"
#include "MonsterPark/Monster/Tag/FMonsterTag.h"
#include "MassActorSubsystem.h"
#include "GameFramework/Actor.h"

UMonsterProcessor::UMonsterProcessor() :EntityQuery(*this)
{

	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UMonsterProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    EntityQuery.AddTagRequirement<FMonsterTag>(EMassFragmentPresence::All);

    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMonsterTargetFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMonsterStatusFragment>(EMassFragmentAccess::ReadWrite);

    EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
}

void UMonsterProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float CurrentTime = Context.GetWorld()->GetTimeSeconds();

    EntityQuery.ForEachEntityChunk(Context, [this, CurrentTime](FMassExecutionContext& Context)
        {
            const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
            auto Velocities = Context.GetMutableFragmentView<FMassVelocityFragment>();
            const TArrayView<FMonsterTargetFragment> SimpleMovementsList = Context.GetMutableFragmentView<FMonsterTargetFragment>();
            const TArrayView<FMonsterStatusFragment> StatusList = Context.GetMutableFragmentView<FMonsterStatusFragment>();

            // 액터 프래그먼트 리스트 가져오기
            const TArrayView<FMassActorFragment> ActorList = Context.GetMutableFragmentView<FMassActorFragment>();
            const bool bHasActorData = ActorList.Num() > 0;

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {
                FTransform& Transform = Transforms[i].GetMutableTransform();
                FVector CurrentLocation = Transform.GetLocation();
                auto& Move = SimpleMovementsList[i];

                // 1. 최초 스폰 시 가장 가까운 노드 설정
                if (Move.CurrentNodeIndex == -1)
                {
                    float MinDist = FLT_MAX;
                    int32 ClosestNode = 0;
                    for (int32 n = 0; n < 17; ++n)
                    {
                        float Dist = FVector::DistSquared(CurrentLocation, NodeLocations[n]);
                        if (Dist < MinDist)
                        {
                            MinDist = Dist;
                            ClosestNode = n;
                        }
                    }
                    Move.PreviousNodeIndex = ClosestNode;

                    TArray<int32> ValidNeighbors;
                    for (int32 j = 0; j < 4; ++j)
                    {
                        if (NodeNeighbors[ClosestNode][j] != -1)
                            ValidNeighbors.Add(NodeNeighbors[ClosestNode][j]);
                    }
                    Move.CurrentNodeIndex = ValidNeighbors[FMath::RandRange(0, ValidNeighbors.Num() - 1)];
                    Move.Target = NodeLocations[Move.CurrentNodeIndex];
                }

                FVector Dir = Move.Target - CurrentLocation;
                Dir.Z = 0.f;
                float DistSq = Dir.SizeSquared();

                // 2. 목표 노드 도달 판정 및 다음 노드 갱신
                if (DistSq < 25200.0f)
                {
                    int32 NodeToPickFrom = Move.CurrentNodeIndex;
                    TArray<int32> ValidNeighbors;

                    for (int32 j = 0; j < 4; ++j)
                    {
                        int32 Neighbor = NodeNeighbors[NodeToPickFrom][j];
                        if (Neighbor != -1 && Neighbor != Move.PreviousNodeIndex)
                        {
                            ValidNeighbors.Add(Neighbor);
                        }
                    }

                    if (ValidNeighbors.Num() > 0)
                    {
                        Move.PreviousNodeIndex = Move.CurrentNodeIndex;
                        Move.CurrentNodeIndex = ValidNeighbors[FMath::RandRange(0, ValidNeighbors.Num() - 1)];
                    }
                    else
                    {
                        int32 Temp = Move.CurrentNodeIndex;
                        Move.CurrentNodeIndex = Move.PreviousNodeIndex;
                        Move.PreviousNodeIndex = Temp;
                    }

                    Move.Target = NodeLocations[Move.CurrentNodeIndex];
                }

                Dir = Move.Target - CurrentLocation;
                Dir.Z = 0.f;

                // 3. ?? 기존 속도 및 회전 연산 코드를 그대로 사용
                if (!Dir.IsNearlyZero())
                {
                    FVector NormalDir = Dir.GetSafeNormal();
                    Velocities[i].Value = NormalDir * StatusList[i].SpeedMultiplier;

                    FRotator CurrentRot = NormalDir.Rotation();
                    CurrentRot.Pitch = 0.f;
                    CurrentRot.Roll = 0.f;
                    Transform.SetRotation(CurrentRot.Quaternion());
                }

                if (bHasActorData)
                {
                    if (AActor* VisualActor = ActorList[i].GetMutable())
                    {
                        TWeakObjectPtr<AActor> WeakActor = VisualActor;
                        FTransform UpdatedTransform = Transform;

                        AsyncTask(ENamedThreads::GameThread, [WeakActor, UpdatedTransform]()
                            {
                                if (WeakActor.IsValid())
                                {
                                    WeakActor->SetActorLocationAndRotation(
                                        UpdatedTransform.GetLocation(),
                                        UpdatedTransform.GetRotation()
                                    );
                                }
                            });
                    }
                }
            }
        });
}