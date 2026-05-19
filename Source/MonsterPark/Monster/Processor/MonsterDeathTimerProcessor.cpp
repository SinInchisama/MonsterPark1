// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterDeathTimerProcessor.h"

#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"

#include "MonsterPark/Monster/Tag/FMonsterTag.h"
#include "MonsterPark/Monster/Tag/KilledTag.h"
#include "MonsterPark/Monster/Tag/MonsterDyingTag.h"

#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"

UMonsterDeathTimerProcessor::UMonsterDeathTimerProcessor() : EntityQuery(*this)
{
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UMonsterDeathTimerProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    EntityQuery.AddTagRequirement<FMonsterDyingTag>(EMassFragmentPresence::All);
    EntityQuery.AddTagRequirement<FKilledTag>(EMassFragmentPresence::None);
    EntityQuery.AddRequirement<FMonsterStatusFragment>(EMassFragmentAccess::ReadWrite);
}

void UMonsterDeathTimerProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
        {
            auto StatusList = Context.GetMutableFragmentView<FMonsterStatusFragment>();
            const float DeltaTime = Context.GetDeltaTimeSeconds();

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {
                StatusList[i].DeathDelayTimer -= DeltaTime;

                if (StatusList[i].DeathDelayTimer <= 0.0f)
                {
                    Context.Defer().AddTag<FKilledTag>(Context.GetEntity(i));
                }
            }
        });
}