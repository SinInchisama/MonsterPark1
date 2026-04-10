// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterHitVFXProcessor.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "NiagaraFunctionLibrary.h"
#include "MassCommonTypes.h"

#include "MonsterPark/Monster/Fragment/FMonsterSharedFragment.h"

#include "MonsterPark/Monster/Tag/MonsterDamagedTag.h"

UMonsterHitVFXProcessor::UMonsterHitVFXProcessor() : HitQuery(*this)
{
	bRequiresGameThreadExecution = true;

	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UMonsterHitVFXProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	HitQuery.AddTagRequirement<FMonsterDamagedTag>(EMassFragmentPresence::All);
	HitQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	HitQuery.AddSharedRequirement<FMonsterRoundSharedFragment>(EMassFragmentAccess::ReadOnly);
}

void UMonsterHitVFXProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	HitQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
		{
			// 1. Shared Fragment에서 나이아가라 이펙트를 가져옴
			const FMonsterRoundSharedFragment& VFXData = Context.GetSharedFragment<FMonsterRoundSharedFragment>();

			UE_LOG(LogTemp, Warning, TEXT("VF12312T"));

			if (!VFXData.HitEffect) return; // 에셋이 없으면 리턴

			UE_LOG(LogTemp, Warning, TEXT("VFX HITEFEECT"));

			// 2. 위치 데이터 뷰
			const TArrayView<const FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
			UWorld* World = Context.GetWorld();

			// 3. 순회하면서 스폰 후 태그 제거
			for (int32 i = 0; i < Context.GetNumEntities(); ++i)
			{
				UE_LOG(LogTemp, Warning, TEXT("VFX HITEFEECT13123123"));
				FVector HitLocation = Transforms[i].GetTransform().GetLocation();

				// 나이아가라 1회성 스폰 (한 번 재생 후 자동 소멸됨)
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, VFXData.HitEffect, HitLocation);

				// 다음 프레임에 또 재생되지 않도록 태그 제거 (Defer 사용)
				Context.Defer().RemoveTag<FMonsterDamagedTag>(Context.GetEntity(i));
			}
		});
}
