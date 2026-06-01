// Fill out your copyright notice in the Description page of Project Settings.


#include "Witch.h"
#include "MonsterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassEntityManager.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MassCommonFragments.h"

void AWitch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWitch::FindEnemiesInArea()
{
	Super::FindEnemiesInArea();

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem || !AbilitySystemComponent)
	{
		return;
	}

	const float RangeValue = AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute());
	const float RadiusSq = FMath::Square(RangeValue);
	FVector MyLocation = GetActorLocation();

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
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

				if (FVector::DistSquared(MyLocation, Transforms[i].GetTransform().GetLocation()) <= RadiusSq)
				{
					Statuses[i].SpeedMultiplier = SlowMultiplier;
					Statuses[i].Defenset = FMath::RoundToInt(Statuses[i].BaseDefense * DefenseReductionMultiplier);
				}
				else
				{
					Statuses[i].SpeedMultiplier = 400.0f;
					Statuses[i].Defenset = Statuses[i].BaseDefense;
				}
			}
		});
}

UAnimMontage* AWitch::GetDetectedMontage() const
{
	return WitchFullMontage;
}

void AWitch::PlayDetectedMontageIfNeeded()
{
	PlayDetectedMontageSection(WitchFullMontage, bHasPlayedPassive);
}
