// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterProcessor.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "FMonsterTargetFragment.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"

UMonsterProcessor::UMonsterProcessor() :EntityQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UMonsterProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    /* Super::ConfigureQueries();

     EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
     EntityQuery.AddRequirement<FSimpleMovementFragment>(EMassFragmentAccess::ReadWrite);*/
     //Super::ConfigureQueries(EntityManager);

    // Super::ConfigureQueries(EntityManager);

    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMonsterTargetFragment>(EMassFragmentAccess::ReadWrite);

    EntityQuery.RegisterWithProcessor(*this);
}

void UMonsterProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float CurrentTime = Context.GetWorld()->GetTimeSeconds();
    const float DeltaTime = Context.GetDeltaTimeSeconds();

    EntityQuery.ForEachEntityChunk(Context, [this, CurrentTime, DeltaTime](FMassExecutionContext& Context)
        {
            const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
            const TArrayView<FMonsterTargetFragment> SimpleMovementsList = Context.GetMutableFragmentView<FMonsterTargetFragment>();
           

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {
                FTransform& Transform = TransformList[i].GetMutableTransform();
                FVector CurrentLocation = Transform.GetLocation();

                auto& Move = SimpleMovementsList[i];

                // SpawnTime 세팅
                if(Move.SpawnTime < 0.f)
                {
                    Move.SpawnTime = CurrentTime + SpawnIndex * 0.5f;
                    SpawnIndex++;
                    Transform.SetLocation(
                        FVector(0, 0, -100000.f));
                    continue;
                }

                if (!Move.bSpawned)
                {
                    /*UE_LOG(LogTemp, Warning,
                        TEXT("[Processor] Now=%.2f SpawnTime=%.2f"),
                        CurrentTime, Move.SpawnTime);*/
                    if (CurrentTime >= Move.SpawnTime)
                    {
                        Move.bSpawned = true;
                        Transform.SetLocation(
                            FVector(-1080, -1080, 60.f));
                    }
                    else
                    {
                        continue;
                    }
                }
                else {
                    // 3 이동 처리
                    FVector Dir = Move.Target - CurrentLocation;

                    if (Dir.SizeSquared() < 10.0f)
                    {
                        Move.TargetIndex = (Move.TargetIndex + 1) % 4;
                        Move.Target = MonsterTargets[Move.TargetIndex];
                        //FMath::RandRange(-1.f, 1.f) * 1000.f
                    }
                    else
                    {
                        Transform.SetLocation(
                            CurrentLocation +
                            Dir.GetSafeNormal() * 400.f * DeltaTime);
                    }
                }
            }
        });
}
