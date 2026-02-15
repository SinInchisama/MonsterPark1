// Fill out your copyright notice in the Description page of Project Settings.


#include "KilledMonster.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "MassObserverProcessor.h"
#include "KilledTag.h"
#include "FMonsterTag.h"

UKilledMonster::UKilledMonster() :EntityQuery(*this)
{
	ObservedType = FKilledTag::StaticStruct();
	Operation = EMassObservedOperation::Add;

	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UKilledMonster::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FMonsterTag>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FKilledTag>(EMassFragmentPresence::All);
}

void UKilledMonster::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	
	/*EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
			for (int i = 0; i < Context.GetNumEntities(); ++i)
			{

			}
	});*/
	//Context.Defer().DestroyEntity(con)
	Context.Defer().DestroyEntities(Context.GetEntities());
}
