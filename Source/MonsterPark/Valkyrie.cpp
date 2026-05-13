// Fill out your copyright notice in the Description page of Project Settings.


#include "Valkyrie.h"
#include "MonsterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PlaySubSystem.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassEntityManager.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MassCommonFragments.h"

void AValkyrie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AValkyrie::FindEnemiesInArea()
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		return;
	}

	const float RangeValue = AbilitySystemComponent
		? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute())
		: DefaultRange;

	const float BaseAttackPowerValue = AbilitySystemComponent
		? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetAttackPowerAttribute())
		: DefaultAttackPower;

	const float AttackPowerValue = BaseAttackPowerValue * AttackBuffMultiplier;

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	FVector MyLocation = GetActorLocation();
	const float RangeSq = FMath::Square(RangeValue);
	const float SplashRadiusSq = FMath::Square(SplashRadius);
	FVector TargetLocation = FVector::ZeroVector;
	bool bFoundTarget = false;

	if (!bIsOutsideWall)
	{
		FMassExecutionContext ExecContext(EntityManager, 0.0f);
		TargetQueryPtr->ForEachEntityChunk(ExecContext, [this, MyLocation, RangeSq, &TargetLocation, &bFoundTarget](FMassExecutionContext& Context)
			{
				if (bFoundTarget)
				{
					return;
				}

				const int32 NumEntities = Context.GetNumEntities();
				TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();

				for (int32 i = 0; i < NumEntities; ++i)
				{
					FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();
					if (FVector::DistSquared(MyLocation, EnemyLoc) <= RangeSq)
					{
						TargetLocation = EnemyLoc;
						bFoundTarget = true;
						return;
					}
				}
			});

		if (bFoundTarget)
		{
			FMassExecutionContext DamageContext(EntityManager, 0.0f);
			TargetQueryPtr->ForEachEntityChunk(DamageContext, [this, TargetLocation, SplashRadiusSq, AttackPowerValue](FMassExecutionContext& Context)
				{
					const int32 NumEntities = Context.GetNumEntities();
					TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
					TArrayView<FMonsterStatusFragment> Statuses = Context.GetMutableFragmentView<FMonsterStatusFragment>();

					for (int32 i = 0; i < NumEntities; ++i)
					{
						FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();
						if (FVector::DistSquared(TargetLocation, EnemyLoc) <= SplashRadiusSq)
						{
							Statuses[i].PendingAoEDamage += AttackPowerValue;
						}
					}
				});
		}
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
					//CandidateMonsters.Append(Cell->MonsterInCell);
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
			if (Transform && FVector::DistSquared(MyLocation, Transform->GetTransform().GetLocation()) <= RangeSq)
			{
				TargetLocation = Transform->GetTransform().GetLocation();
				bFoundTarget = true;
				break;
			}
		}

		if (bFoundTarget)
		{
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
					float DistSq = FVector::DistSquared(TargetLocation, Transform->GetTransform().GetLocation());
					if (DistSq <= SplashRadiusSq)
					{
						Status->PendingAoEDamage += AttackPowerValue;
					}
				}
			}
		}
	}

	if (bFoundTarget)
	{
		bEnemyDetected = true;
		Attacking = false;

		FVector Direction = (TargetLocation - MyLocation).GetSafeNormal2D();
		if (!Direction.IsNearlyZero())
		{
			SetActorRotation(Direction.Rotation());
		}
	}
}

UAnimMontage* AValkyrie::GetDetectedMontage() const
{
	return ValkyrieFullMontage;
}

void AValkyrie::PlayDetectedMontageIfNeeded()
{
	if (!bEnemyDetected)
	{
		return;
	}

	if (USkeletalMeshComponent* CharacterMesh = FindComponentByClass<USkeletalMeshComponent>())
	{
		if (UAnimInstance* AnimInst = CharacterMesh->GetAnimInstance())
		{
			if (ValkyrieFullMontage && !AnimInst->Montage_IsPlaying(ValkyrieFullMontage))
			{
				AnimInst->Montage_Play(ValkyrieFullMontage);

				if (!bHasPlayedPassive)
				{
					AnimInst->Montage_JumpToSection(FName("Passive"), ValkyrieFullMontage);
					bHasPlayedPassive = true;
				}
				else
				{
					AnimInst->Montage_JumpToSection(FName("Attack"), ValkyrieFullMontage);
				}
			}
		}
	}
}
