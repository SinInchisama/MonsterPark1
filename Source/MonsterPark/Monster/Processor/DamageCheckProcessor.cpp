// Fill out your copyright notice in the Description page of Project Settings.

#include "DamageCheckProcessor.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassCommonTypes.h"
#include "MassCommonFragments.h"
#include "GameFramework/Actor.h"
#include "MonsterPark/Monster/EntityHealthInterface.h"

#include "MonsterPark/Monster/Fragment/FMonsterConditionFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterTargetFragment.h"

#include "MonsterPark/Monster/Tag/FMonsterTag.h"
#include "MonsterPark/Monster/Tag/KilledTag.h"
#include "MonsterPark/Monster/Tag/MonsterDamagedTag.h"
#include "MonsterPark/Monster/Tag/MonsterDyingTag.h"

#include "MassRepresentationFragments.h"
#include "MassActorSubsystem.h"

UDamageCheckProcessor::UDamageCheckProcessor() : DamageCheckQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;

	// UI 업데이트(AActor 접근)를 위해 메인 스레드 실행 강제
	bRequiresGameThreadExecution = true;
}

void UDamageCheckProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	DamageCheckQuery.AddTagRequirement<FKilledTag>(EMassFragmentPresence::None);
	DamageCheckQuery.AddTagRequirement<FMonsterDyingTag>(EMassFragmentPresence::None);

	DamageCheckQuery.AddRequirement<FMonsterConditionFragment>(EMassFragmentAccess::ReadWrite);
	DamageCheckQuery.AddRequirement<FMonsterStatusFragment>(EMassFragmentAccess::ReadWrite);
	DamageCheckQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);

	DamageCheckQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
}

void UDamageCheckProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	DamageCheckQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			const TArrayView<FMonsterConditionFragment> ConditionList = Context.GetMutableFragmentView<FMonsterConditionFragment>();
			const TArrayView<FMonsterStatusFragment> StatusList = Context.GetMutableFragmentView<FMonsterStatusFragment>();

			// FMassActorFragment 리스트 가져오기
			const TArrayView<FMassActorFragment> ActorList = Context.GetMutableFragmentView<FMassActorFragment>();

			const int32 NumEntities = Context.GetNumEntities();
			const bool bHasActorData = ActorList.Num() > 0;

			for (int32 i = 0; i < NumEntities; ++i)
			{
				float TotalDamageThisFrame = 0.0f;

				// 1. 대미지 합산 로직
				if (ConditionList[i].Damage != 0)
				{
					TotalDamageThisFrame += ConditionList[i].Damage;
					ConditionList[i].Damage = 0;
				}
				if (StatusList[i].PendingAoEDamage != 0)
				{
					TotalDamageThisFrame += StatusList[i].PendingAoEDamage;
					StatusList[i].PendingAoEDamage = 0;
				}
				if (StatusList[i].PendingBleedDamage != 0)
				{
					TotalDamageThisFrame += StatusList[i].PendingBleedDamage;
					StatusList[i].PendingBleedDamage = 0;
				}

				if (TotalDamageThisFrame > 0.0f)
				{
					StatusList[i].CurrentHealth -= TotalDamageThisFrame;
					Context.Defer().AddTag<FMonsterDamagedTag>(Context.GetEntity(i));

					if (bHasActorData)
					{
						AActor* VisualActor = ActorList[i].GetMutable();

						if (VisualActor != nullptr && VisualActor->Implements<UEntityHealthInterface>())
						{
							IEntityHealthInterface::Execute_UpdateHealthUI(VisualActor, StatusList[i].CurrentHealth);
						}
					}
				}

				if (StatusList[i].CurrentHealth <= 0)
				{
					Context.Defer().AddTag<FMonsterDyingTag>(Context.GetEntity(i));
				}
			}
		});
}