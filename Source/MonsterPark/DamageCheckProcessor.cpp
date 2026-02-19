// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageCheckProcessor.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"

#include "FMonsterConditionFragment.h"
#include "FMonsterStatusFragment.h"
#include "MassCommonFragments.h"
#include "FMonsterTargetFragment.h"

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

	DamageCheckQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite); // static mesh를 지우는 방법을 몰라서 일단 위치 변경으로 안 보이게 만듬
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
					// UE_LOG(LogTemp, Warning, TEXT("Detected Count: %f"), StatusList[i].CurrentHealth);
					
				}
				if (StatusList[i].CurrentHealth <= 0) {
					if (!SimpleMovementsList[i].Death) {                // 임시로 위치 변경 시켜버림, 추후 static mesh 없애는 방법 알게 되면 수정 예정
						FTransform& Transform = Transforms[i].GetMutableTransform();
						Transform.SetLocation(
							FVector(0, 0, -10000.f));
						SimpleMovementsList[i].Death = true;
					}
					else {
						Context.Defer().AddTag<FKilledTag>(Context.GetEntity(i));
					}
				}
			}

		});
}
