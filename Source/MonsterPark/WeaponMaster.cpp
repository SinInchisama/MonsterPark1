// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponMaster.h"
#include "WeaponMasterSkillAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "MonsterAttributeSet.h"
#include "PlaySubSystem.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassEntityManager.h"
#include "MonsterPark/Monster/Fragment/FMonsterConditionFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"

AWeaponMaster::AWeaponMaster()
{
	SkillAbilityClass = UWeaponMasterSkillAbility::StaticClass();
}

void AWeaponMaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeaponMaster::FindEnemiesInArea()
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		return;
	}

	const float RangeValue = AbilitySystemComponent
		? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute())
		: DefaultRange;

	const float AttackPowerValue = AbilitySystemComponent
		? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetAttackPowerAttribute())
		: DefaultAttackPower;

	const float CriticalRoll = FMath::FRandRange(0.0f, 1.0f);
	const float DamageMultiplier = (CriticalRoll <= CriticalChance) ? CriticalDamageMultiplier : 1.0f;
	const float FinalAttackPower = AttackPowerValue * DamageMultiplier;

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
			TargetQueryPtr->ForEachEntityChunk(DamageContext, [this, TargetLocation, SplashRadiusSq, FinalAttackPower](FMassExecutionContext& Context)
				{
					const int32 NumEntities = Context.GetNumEntities();
					TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
					TArrayView<FMonsterStatusFragment> Statuses = Context.GetMutableFragmentView<FMonsterStatusFragment>();

					for (int32 i = 0; i < NumEntities; ++i)
					{
						FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();
						if (FVector::DistSquared(TargetLocation, EnemyLoc) <= SplashRadiusSq)
						{
							Statuses[i].PendingAoEDamage += FinalAttackPower;
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
						Status->PendingAoEDamage += FinalAttackPower;
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

bool AWeaponMaster::ExecuteSkill()
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem || !TargetQueryPtr)
	{
		return false;
	}

	const float RangeValue = AbilitySystemComponent
		? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute())
		: DefaultRange;

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	FVector MyLocation = GetActorLocation();
	const float RadiusSq = FMath::Square(RangeValue);
	bool bApplied = false;

	if (!bIsOutsideWall)
	{
		FMassExecutionContext ExecContext(EntityManager, 0.0f);
		TargetQueryPtr->ForEachEntityChunk(ExecContext, [this, MyLocation, RadiusSq, &bApplied](FMassExecutionContext& Context)
			{
				if (bApplied)
				{
					return;
				}

				const int32 NumEntities = Context.GetNumEntities();
				TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
				TArrayView<FMonsterConditionFragment> Conditions = Context.GetMutableFragmentView<FMonsterConditionFragment>();

				for (int32 i = 0; i < NumEntities; ++i)
				{
					FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();
					if (FVector::DistSquared(MyLocation, EnemyLoc) <= RadiusSq)
					{
						Conditions[i].Damage += SkillDamage;
						bApplied = true;
						return;
					}
				}
			});
	}
	else
	{
		if (!PlaySubsystem)
		{
			return false;
		}

		int64 MyKey = PlaySubsystem->GetGridKey(MyLocation);
		int32 CenterX = (int32)(MyKey >> 32);
		int32 CenterY = (int32)(MyKey & 0xFFFFFFFF);

		bool bFoundInGrid = false;
		for (int32 x = -1; x <= 1; ++x)
		{
			if (bFoundInGrid)
			{
				break;
			}

			for (int32 y = -1; y <= 1; ++y)
			{
				int64 CheckKey = ((int64)(CenterX + x) << 32) | (uint32)(CenterY + y);

				if (FGridData* Cell = PlaySubsystem->SpatialGrid.Find(CheckKey))
				{
					for (const FMonsterGridInfo& MInfo : Cell->MonsterInfos)
					{
						if (FVector::DistSquared(MyLocation, MInfo.Location) <= RadiusSq)
						{
							if (!EntityManager.IsEntityValid(MInfo.MonsterHandle))
							{
								continue;
							}

							FMassEntityView EntityView(EntityManager, MInfo.MonsterHandle);
							if (FMonsterConditionFragment* Condition = EntityView.GetFragmentDataPtr<FMonsterConditionFragment>())
							{
								Condition->Damage += SkillDamage;
								bApplied = true;
								bFoundInGrid = true;
								break;
							}
						}
					}
				}

				if (bFoundInGrid)
				{
					break;
				}
			}
		}
	}

	return bApplied;
}

void AWeaponMaster::UseSkill()
{
	if (!AbilitySystemComponent || !SkillAbilityClass)
	{
		return;
	}

	FGameplayAbilitySpec* SkillSpec = AbilitySystemComponent->FindAbilitySpecFromClass(SkillAbilityClass);
	UWeaponMasterSkillAbility* SkillAbility = SkillSpec
		? Cast<UWeaponMasterSkillAbility>(SkillSpec->GetPrimaryInstance())
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

	UAnimMontage* MontageToPlay = SkillMontage ? SkillMontage : WeaponMasterSkillMontage_DEPRECATED;
	if (MontageToPlay)
	{
		if (USkeletalMeshComponent* CharacterMesh = GetMesh())
		{
			if (UAnimInstance* AnimInst = CharacterMesh->GetAnimInstance())
			{
				AnimInst->Montage_Play(MontageToPlay);
				AnimInst->Montage_JumpToSection(FName("Skill"), MontageToPlay);
			}
		}
	}
}

bool AWeaponMaster::IsSkillRequested() const
{
	return bSkillRequested;
}

void AWeaponMaster::ClearSkillRequest()
{
	bSkillRequested = false;
}

UAnimMontage* AWeaponMaster::GetDetectedMontage() const
{
	return WeaponMasterFullMontage;
}

void AWeaponMaster::PlayDetectedMontageIfNeeded()
{
	if (!bEnemyDetected)
	{
		return;
	}

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (CharacterMesh)
	{
		if (UAnimInstance* AnimInst = CharacterMesh->GetAnimInstance())
		{
			if (WeaponMasterFullMontage && !AnimInst->Montage_IsPlaying(WeaponMasterFullMontage))
			{
				AnimInst->Montage_Play(WeaponMasterFullMontage);

				if (!bHasPlayedPassive)
				{
					AnimInst->Montage_JumpToSection(FName("Passive"), WeaponMasterFullMontage);
					bHasPlayedPassive = true;
				}
				else
				{
					AnimInst->Montage_JumpToSection(FName("Attack"), WeaponMasterFullMontage);
				}
			}
		}
	}
}

