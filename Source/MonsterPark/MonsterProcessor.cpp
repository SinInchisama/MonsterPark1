// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterProcessor.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "FMonsterTargetFragment.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassMovementFragments.h"
#include "FMonsterTag.h"

UMonsterProcessor::UMonsterProcessor() :EntityQuery(*this)
{

	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UMonsterProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
    EntityQuery.AddTagRequirement<FMonsterTag>(EMassFragmentPresence::All);

    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMonsterTargetFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    
}

void UMonsterProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float CurrentTime = Context.GetWorld()->GetTimeSeconds();

    EntityQuery.ForEachEntityChunk(Context, [this, CurrentTime](FMassExecutionContext& Context)
        {
            const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
            auto Velocities = Context.GetMutableFragmentView<FMassVelocityFragment>();
            const TArrayView<FMonsterTargetFragment> SimpleMovementsList = Context.GetMutableFragmentView<FMonsterTargetFragment>();
           

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {
                FTransform& Transform = Transforms[i].GetMutableTransform();
                FVector CurrentLocation = Transform.GetLocation();

                auto& Move = SimpleMovementsList[i];
      
               FVector Dir = Move.Target - CurrentLocation;

               if (Dir.SizeSquared() < 10.0f)
               {
                   Move.TargetIndex = (Move.TargetIndex + 1) % 4;
                   Move.Target = MonsterTargets[Move.TargetIndex];
               }
               else
               {

                   FVector NormalDir = Dir.GetSafeNormal();

                   FRotator CurrentRot = NormalDir.Rotation();

                   CurrentRot.Pitch = 0.f;
                   CurrentRot.Roll = 0.f;
                   Transform.SetRotation(CurrentRot.Quaternion());

                   Velocities[i].Value = Dir.GetSafeNormal() * 400.f;
               }
               
            }
        });
}
