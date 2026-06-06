// Fill out your copyright notice in the Description page of Project Settings.


#include "Sage.h"
#include "SageSkillAbility.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassEntityManager.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MassCommonFragments.h"
#include "Animation/AnimInstance.h"

// [추가된 헤더들] 죽은 몬스터 판별 및 격자 탐색에 필요
#include "MassEntityView.h"
#include "MonsterPark/Monster/Tag/KilledTag.h"
#include "MonsterPark/Monster/Tag/MonsterDyingTag.h"
#include "PlaySubSystem.h" 

ASage::ASage()
{
	SkillAbilityClass = USageSkillAbility::StaticClass();
}

void ASage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASage::FindEnemiesInArea()
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem || !PlaySubsystem)
	{
		return;
	}

	const float RangeValue = AbilitySystemComponent
		? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute())
		: DefaultRange;

	const float AttackPowerValue = AbilitySystemComponent
		? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetAttackPowerAttribute())
		: DefaultAttackPower;

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
		// [수정된 부분] 성벽 밖에 있을 때 격자(Grid) 시스템을 통한 탐색 로직 추가
		int64 MyKey = PlaySubsystem->GetGridKey(MyLocation);
		int32 CenterX = (int32)(MyKey >> 32);
		int32 CenterY = (int32)(MyKey & 0xFFFFFFFF);

		for (int32 x = -1; x <= 1; ++x)
		{
			if (bFoundTarget) break;
			for (int32 y = -1; y <= 1; ++y)
			{
				int64 CheckKey = ((int64)(CenterX + x) << 32) | (uint32)(CenterY + y);
				if (FGridData* Cell = PlaySubsystem->SpatialGrid.Find(CheckKey))
				{
					for (const FMonsterGridInfo& MInfo : Cell->MonsterInfos)
					{
						if (FVector::DistSquared(MyLocation, MInfo.Location) <= RangeSq)
						{
							if (!EntityManager.IsEntityValid(MInfo.MonsterHandle)) continue;
							FMassEntityView View(EntityManager, MInfo.MonsterHandle);
							if (View.HasTag<FKilledTag>() || View.HasTag<FMonsterDyingTag>()) continue;

							TargetLocation = MInfo.Location;
							bFoundTarget = true;
							break;
						}
					}
				}
				if (bFoundTarget) break;
			}
		}

		if (bFoundTarget)
		{
			int64 TargetKey = PlaySubsystem->GetGridKey(TargetLocation);
			int32 TargetCenterX = (int32)(TargetKey >> 32);
			int32 TargetCenterY = (int32)(TargetKey & 0xFFFFFFFF);

			for (int32 x = -1; x <= 1; ++x)
			{
				for (int32 y = -1; y <= 1; ++y)
				{
					int64 CheckKey = ((int64)(TargetCenterX + x) << 32) | (uint32)(TargetCenterY + y);
					if (FGridData* Cell = PlaySubsystem->SpatialGrid.Find(CheckKey))
					{
						for (const FMonsterGridInfo& MInfo : Cell->MonsterInfos)
						{
							if (FVector::DistSquared(TargetLocation, MInfo.Location) <= SplashRadiusSq)
							{
								if (!EntityManager.IsEntityValid(MInfo.MonsterHandle)) continue;
								FMassEntityView View(EntityManager, MInfo.MonsterHandle);
								if (View.HasTag<FKilledTag>() || View.HasTag<FMonsterDyingTag>()) continue;

								if (FMonsterStatusFragment* Status = View.GetFragmentDataPtr<FMonsterStatusFragment>())
								{
									Status->PendingAoEDamage += AttackPowerValue;
								}
							}
						}
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

	ApplyStun();
}

void ASage::UseSkill()
{
	if (!AbilitySystemComponent || !SkillAbilityClass)
	{
		return;
	}

	FGameplayAbilitySpec* SkillSpec = AbilitySystemComponent->FindAbilitySpecFromClass(SkillAbilityClass);
	USageSkillAbility* SkillAbility = SkillSpec
		? Cast<USageSkillAbility>(SkillSpec->GetPrimaryInstance())
		: nullptr;
	if (!SkillAbility || !SkillAbility->IsCooldownReady(GetWorld()))
	{
		return;
	}

	bSkillRequested = true;

	const bool bActivated = AbilitySystemComponent->TryActivateAbilityByClass(SkillAbilityClass);
	if (!bActivated)
	{
		bSkillRequested = false;
		return;
	}

	if (SkillMontage)
	{
		if (USkeletalMeshComponent* CharacterMesh = GetMesh())
		{
			if (UAnimInstance* AnimInst = CharacterMesh->GetAnimInstance())
			{
				AnimInst->Montage_Play(SkillMontage);
				AnimInst->Montage_JumpToSection(FName("Skill"), SkillMontage);
			}
		}
	}
}

bool ASage::ExecuteSkill()
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		return false;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FVector MyLocation = GetActorLocation();
	const float SkillRadiusSq = FMath::Square(SkillRadius);

	FMassExecutionContext ExecContext(EntityManager, 0.0f);
	TargetQueryPtr->ForEachEntityChunk(ExecContext, [this, MyLocation, SkillRadiusSq](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();
			TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
			TArrayView<FMonsterStatusFragment> Statuses = Context.GetMutableFragmentView<FMonsterStatusFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();
				if (FVector::DistSquared(MyLocation, EnemyLoc) <= SkillRadiusSq)
				{
					Statuses[i].PendingAoEDamage += SkillDamage;
				}
			}
		});

	return true;
}

bool ASage::IsSkillRequested() const
{
	return bSkillRequested;
}

void ASage::ClearSkillRequest()
{
	bSkillRequested = false;
}

UAnimMontage* ASage::GetDetectedMontage() const
{
	return SageFullMontage;
}

void ASage::PlayDetectedMontageIfNeeded()
{
	PlayDetectedMontageSection(SageFullMontage, bHasPlayedPassive);
}

void ASage::ApplyStun()
{
	if (GetWorldTimerManager().IsTimerActive(StunResetHandle))
	{
		return;
	}

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FVector MyLocation = GetActorLocation();
	const float StunRadiusSq = FMath::Square(StunRadius);

	StunnedEntities.Reset();
	FMassExecutionContext ExecContext(EntityManager, 0.0f);
	EnemyQuery.ForEachEntityChunk(ExecContext, [this, MyLocation, StunRadiusSq](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();
			auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();
			auto Statuses = Context.GetMutableFragmentView<FMonsterStatusFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				if (FVector::DistSquared(MyLocation, Transforms[i].GetTransform().GetLocation()) <= StunRadiusSq)
				{
					Statuses[i].SpeedMultiplier = StunSpeedMultiplier;
					StunnedEntities.Add(Context.GetEntity(i));
				}
			}
		});

	if (StunnedEntities.Num() > 0)
	{
		GetWorldTimerManager().SetTimer(StunResetHandle, this, &ASage::ResetStun, StunDuration, false);
	}
}

void ASage::ResetStun()
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		StunnedEntities.Reset();
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	for (const FMassEntityHandle& EntityHandle : StunnedEntities)
	{
		if (!EntityManager.IsEntityValid(EntityHandle))
		{
			continue;
		}

		FMonsterStatusFragment* Status = EntityManager.GetFragmentDataPtr<FMonsterStatusFragment>(EntityHandle);
		if (Status)
		{
			Status->SpeedMultiplier = 400.0f;
		}
	}

	StunnedEntities.Reset();
}