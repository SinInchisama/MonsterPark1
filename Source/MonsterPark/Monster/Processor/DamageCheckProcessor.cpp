// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageCheckProcessor.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"

#include "MonsterPark/Monster/Fragment/FMonsterConditionFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MassCommonFragments.h"
#include "MonsterPark/Monster/Fragment/FMonsterTargetFragment.h"

#include "MonsterPark/Monster/Tag/FMonsterTag.h"
#include "MonsterPark/Monster/Tag/KilledTag.h"

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

	DamageCheckQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite); // static mesh�� ����� ����� ��� �ϴ� ��ġ �������� �� ���̰� ����
	DamageCheckQuery.AddRequirement<FMonsterTargetFragment>(EMassFragmentAccess::ReadWrite);
}

void UDamageCheckProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	DamageCheckQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			const TArrayView<FMonsterConditionFragment> ConditionList = Context.GetMutableFragmentView<FMonsterConditionFragment>();
			const TArrayView<FMonsterStatusFragment> StatusList = Context.GetMutableFragmentView<FMonsterStatusFragment>();

			const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
			const TArrayView<FMonsterTargetFragment> SimpleMovementsList = Context.GetMutableFragmentView<FMonsterTargetFragment>();

			for (int32 i = 0; i < Context.GetNumEntities(); ++i)
			{
				if (ConditionList[i].Damage != 0)
				{
					StatusList[i].CurrentHealth -= ConditionList[i].Damage;
					ConditionList[i].Damage = 0;
					
				}
				if (StatusList[i].CurrentHealth <= 0) {
					Context.Defer().AddTag<FKilledTag>(Context.GetEntity(i));
				}
			}

		});
}
