// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterDyingObserver.h"
#include "MassCommonTypes.h"
#include "MassRepresentationFragments.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MonsterPark/Monster/Tag/FMonsterTag.h"
#include "MonsterPark/Monster/AttackVisualActor.h"
#include "MonsterPark/Monster/Tag/MonsterDyingTag.h"
#include "MassActorSubsystem.h"

UMonsterDyingObserver::UMonsterDyingObserver() : EntityQuery(*this)
{
	ObservedType = FMonsterDyingTag::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UMonsterDyingObserver::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMonsterStatusFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
}

void UMonsterDyingObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
        {
            auto StatusList = Context.GetMutableFragmentView<FMonsterStatusFragment>();
            auto ActorList = Context.GetMutableFragmentView<FMassActorFragment>();
            bool bHasActor = ActorList.Num() > 0;

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {
                if (bHasActor && ActorList[i].IsValid())
                {
                    AActor* VisualActor = ActorList[i].GetMutable();

                    AsyncTask(ENamedThreads::GameThread, [VisualActor, StatusList,i]()
                        {
                            if (AAttackVisualActor* AnimActor = Cast<AAttackVisualActor>(VisualActor))
                            {
                                AnimActor->PlayDeathAnimation();
                            }
                            else
                            {
                                StatusList[i].DeathDelayTimer = 0.0f;
                            }
                        });
                }
                else
                {
                    StatusList[i].DeathDelayTimer = 0.0f;
                }
            }
        });
}