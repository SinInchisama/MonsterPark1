// Fill out your copyright notice in the Description page of Project Settings.


#include "Lancer.h"
#include "MonsterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PlaySubSystem.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassEntityManager.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MassCommonFragments.h"

void ALancer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALancer::FindEnemiesInArea()
{
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

	if (!bIsOutsideWall)
	{
		FMassExecutionContext ExecContext(EntityManager, 0.0f);
		TargetQueryPtr->ForEachEntityChunk(ExecContext, [this, MyLocation, RadiusSq](FMassExecutionContext& Context)
			{
				const int32 NumEntities = Context.GetNumEntities();
				TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
				TArrayView<FMonsterStatusFragment> Statuses = Context.GetMutableFragmentView<FMonsterStatusFragment>();

				for (int32 i = 0; i < NumEntities; ++i)
				{
					FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();
					if (FVector::DistSquared(MyLocation, EnemyLoc) <= RadiusSq)
					{
						Statuses[i].PendingBleedDamage += Statuses[i].MaxHealt * BleedPercent;
						bEnemyDetected = true;
						Attacking = false;

						FVector Direction = (EnemyLoc - MyLocation).GetSafeNormal2D();
						if (!Direction.IsNearlyZero())
						{
							SetActorRotation(Direction.Rotation());
						}
						return;
					}
				}
			});
	}
	else
	{
		int64 MyKey = PlaySubsystem ? PlaySubsystem->GetGridKey(MyLocation) : -1;
		if (MyKey == -1)
		{
			return;
		}

		TArray<FMassEntityHandle> CandidateMonsters;
		int32 CenterX = (int32)(MyKey >> 32);
		int32 CenterY = (int32)(MyKey & 0xFFFFFFFF);

		for (int32 x = -1; x <= 1; ++x)
		{
			for (int32 y = -1; y <= 1; ++y)
			{
				int64 CheckKey = ((int64)(CenterX + x) << 32) | (uint32)(CenterY + y);
				if (FGridData* Cell = PlaySubsystem->SpatialGrid.Find(CheckKey))
				{
					CandidateMonsters.Append(Cell->MonsterInCell);
				}
			}
		}

		for (FMassEntityHandle MonsterHandle : CandidateMonsters)
		{
			if (!EntityManager.IsEntityValid(MonsterHandle))
			{
				continue;
			}

			FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(MonsterHandle);
			FMonsterStatusFragment* Status = EntityManager.GetFragmentDataPtr<FMonsterStatusFragment>(MonsterHandle);
			if (Transform && Status)
			{
				FVector EnemyLoc = Transform->GetTransform().GetLocation();
				float DistSq = FVector::DistSquared(MyLocation, EnemyLoc);
				if (DistSq <= RadiusSq)
				{
					Status->PendingBleedDamage += Status->MaxHealt * BleedPercent;
					bEnemyDetected = true;
					Attacking = false;

					FVector Direction = (EnemyLoc - MyLocation).GetSafeNormal2D();
					if (!Direction.IsNearlyZero())
					{
						SetActorRotation(Direction.Rotation());
					}
					break;
				}
			}
		}
	}
}

UAnimMontage* ALancer::GetDetectedMontage() const
{
	return LancerFullMontage;
}
