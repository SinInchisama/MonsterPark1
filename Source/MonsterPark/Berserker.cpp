// Fill out your copyright notice in the Description page of Project Settings.


#include "Berserker.h"
#include "MonsterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassEntityManager.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MassCommonFragments.h"

void ABerserker::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
};

void ABerserker::FindEnemiesInArea()
{
	Super::FindEnemiesInArea();

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		return;
	}

	const float RangeValue = AbilitySystemComponent
		? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute())
		: DefaultRange;

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	FVector MyLocation = GetActorLocation();
	const float RadiusSq = FMath::Square(RangeValue);

	FMassExecutionContext ExecContext(EntityManager, 0.0f);
	EnemyQuery.ForEachEntityChunk(ExecContext, [this, MyLocation, RadiusSq](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();
			TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
			TArrayView<FMonsterStatusFragment> Statuses = Context.GetMutableFragmentView<FMonsterStatusFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				if (Statuses[i].BaseDefense <= 0)
				{
					Statuses[i].BaseDefense = Statuses[i].Defenset;
				}

				FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();
				if (FVector::DistSquared(MyLocation, EnemyLoc) <= RadiusSq)
				{
					Statuses[i].Defenset = FMath::RoundToInt(Statuses[i].BaseDefense * DefenseReductionMultiplier);
				}
				else
				{
					Statuses[i].Defenset = Statuses[i].BaseDefense;
				}
			}
		});
}

UAnimMontage* ABerserker::GetDetectedMontage() const
{
    return BerserkerFullMontage;
}