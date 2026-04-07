// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterSpawnProcessor.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "MonsterPark/Monster/Tag/FMonsterTag.h"
#include "MonsterPark/Monster/Tag/MonsterSpawnTag.h"
#include "MonsterPark/Monster/Fragment/FMonsterTargetFragment.h"
#include "MassCommonFragments.h"
#include "MonsterPark/Monster/Fragment/FMonsterSharedFragment.h"

UMonsterSpawnProcessor::UMonsterSpawnProcessor() :EntityQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UMonsterSpawnProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FMonsterSpawnTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FMonsterTag>(EMassFragmentPresence::None);

    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite); 
    EntityQuery.AddSharedRequirement<FMonsterRoundSharedFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMonsterTargetFragment>(EMassFragmentAccess::ReadWrite);
}

void UMonsterSpawnProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float CurrentTime = Context.GetWorld()->GetTimeSeconds();

    EntityQuery.ForEachEntityChunk(Context, [this, CurrentTime](FMassExecutionContext& Context)
        {
            const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
            const TArrayView<FMonsterTargetFragment> SimpleMovementsList = Context.GetMutableFragmentView<FMonsterTargetFragment>();
            FMonsterRoundSharedFragment& RoundData = Context.GetMutableSharedFragment<FMonsterRoundSharedFragment>();

            RoundData.NextSpawnTime = CurrentTime;

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {

                FTransform& Transform = Transforms[i].GetMutableTransform();
                FVector CurrentLocation = Transform.GetLocation();

                auto& Move = SimpleMovementsList[i];

                if (Move.SpawnTime < 0.f)
                {
                    Move.SpawnTime = RoundData.NextSpawnTime + i * 0.5f;
                    Transform.SetLocation(
                        FVector(0, 0, -100000.f));
                    continue;
                }

                if (!Move.bSpawned)
                {
                    if (CurrentTime >= Move.SpawnTime)
                    {
                        Move.bSpawned = true;
                        Transform.SetLocation(
                            FVector(-2600, 400, 60.f));
                        Context.Defer().AddTag<FMonsterTag>(Context.GetEntity(i));
                        Context.Defer().RemoveTag<FMonsterSpawnTag>(Context.GetEntity(i));
                    }
                    else
                    {
                        continue;
                    }
                }

            }
        });
}
