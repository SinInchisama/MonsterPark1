// Fill out your copyright notice in the Description page of Project Settings.


#include "KilledMonster.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "MassObserverProcessor.h"
#include "MonsterPark/Monster/Tag/KilledTag.h"
#include "MonsterPark/Monster/Tag/FMonsterTag.h"

UKilledMonster::UKilledMonster() :EntityQuery(*this)
{
	ObservedType = FKilledTag::StaticStruct();
	Operation = EMassObservedOperation::Add;

    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UKilledMonster::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FKilledTag>(EMassFragmentPresence::All);
}

void UKilledMonster::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
        {

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {
                Context.Defer().DestroyEntity(Context.GetEntity(i));

            }
        });
}
