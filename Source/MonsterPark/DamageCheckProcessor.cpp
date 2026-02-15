// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageCheckProcessor.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"

#include "FMonsterConditionFragment.h"
#include "FMonsterStatusFragment.h"

#include "FMonsterTag.h"
#include "KilledTag.h"

UDamageCheckProcessor::UDamageCheckProcessor() :DamageCheckQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UDamageCheckProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	DamageCheckQuery.AddTagRequirement<FMonsterTag>(EMassFragmentPresence::All);
	DamageCheckQuery.AddTagRequirement<FKilledTag>(EMassFragmentPresence::None);

	DamageCheckQuery.AddRequirement<FMonsterConditionFragment>(EMassFragmentAccess::ReadWrite);
	DamageCheckQuery.AddRequirement<FMonsterStatusFragment>(EMassFragmentAccess::ReadWrite);

}

void UDamageCheckProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	DamageCheckQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			const TArrayView<FMonsterConditionFragment> ConditionList = Context.GetMutableFragmentView<FMonsterConditionFragment>();
			const TArrayView<FMonsterStatusFragment> StatusList = Context.GetMutableFragmentView<FMonsterStatusFragment>();

			for (int32 i = 0; i < Context.GetNumEntities(); ++i)
			{
				if (ConditionList[i].Damage != 0)
				{
					StatusList[i].CurrentHealth -= ConditionList[i].Damage;
					ConditionList[i].Damage = 0;
					UE_LOG(LogTemp, Warning, TEXT("Detected Count: %f"), StatusList[i].CurrentHealth);
					if (StatusList[i].CurrentHealth <= 0) {
						//Context.Defer().AddTag<FKilledTag>(Context.GetEntity(i));
						Context.Defer().DestroyEntity(Context.GetEntity(i));
					}
				}
			}

		});
}
