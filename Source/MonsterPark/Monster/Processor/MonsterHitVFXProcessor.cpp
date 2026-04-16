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

			const FMonsterRoundSharedFragment& VFXData = Context.GetSharedFragment<FMonsterRoundSharedFragment>();

			if (!VFXData.HitEffect) return; 

			const TArrayView<const FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
			UWorld* World = Context.GetWorld();


			for (int32 i = 0; i < Context.GetNumEntities(); ++i)
			{
				FVector HitLocation = Transforms[i].GetTransform().GetLocation();
				HitLocation.Z += 30.0f;

				UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, VFXData.HitEffect, HitLocation);

				Context.Defer().RemoveTag<FMonsterDamagedTag>(Context.GetEntity(i));
			}
		});
}
