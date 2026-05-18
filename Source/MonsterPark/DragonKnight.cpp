// Fill out your copyright notice in the Description page of Project Settings.


#include "DragonKnight.h"
#include "DragonKnightSkillAbility.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassEntityManager.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MassCommonFragments.h"
#include "Animation/AnimInstance.h"

ADragonKnight::ADragonKnight()
{
	SkillAbilityClass = UDragonKnightSkillAbility::StaticClass();
}

void ADragonKnight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADragonKnight::FindEnemiesInArea()
{
	Super::FindEnemiesInArea();
}

void ADragonKnight::UseSkill()
{
	if (!AbilitySystemComponent || !SkillAbilityClass)
	{
		return;
	}

	FGameplayAbilitySpec* SkillSpec = AbilitySystemComponent->FindAbilitySpecFromClass(SkillAbilityClass);
	if (!SkillSpec && HasAuthority())
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(SkillAbilityClass, 1, 1, this));
		SkillSpec = AbilitySystemComponent->FindAbilitySpecFromClass(SkillAbilityClass);
	}
	UDragonKnightSkillAbility* SkillAbility = SkillSpec
		? Cast<UDragonKnightSkillAbility>(SkillSpec->GetPrimaryInstance())
		: nullptr;
	if (SkillAbility && !SkillAbility->IsCooldownReady(GetWorld()))
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

bool ADragonKnight::ExecuteSkill()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	StartBreathDamage();
	return true;
}

bool ADragonKnight::IsSkillRequested() const
{
	return bSkillRequested;
}

void ADragonKnight::ClearSkillRequest()
{
	bSkillRequested = false;
}

UAnimMontage* ADragonKnight::GetDetectedMontage() const
{
	return DragonKnightFullMontage;
}

void ADragonKnight::PlayDetectedMontageIfNeeded()
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
			if (DragonKnightFullMontage && !AnimInst->Montage_IsPlaying(DragonKnightFullMontage))
			{
				AnimInst->Montage_Play(DragonKnightFullMontage);

				if (!bHasPlayedPassive)
				{
					AnimInst->Montage_JumpToSection(FName("Passive"), DragonKnightFullMontage);
					bHasPlayedPassive = true;
				}
				else
				{
					AnimInst->Montage_JumpToSection(FName("Attack"), DragonKnightFullMontage);
				}
			}
		}
	}
}

void ADragonKnight::StartBreathDamage()
{
	if (BreathTickInterval <= 0.0f)
	{
		return;
	}

	TotalBreathTicks = FMath::Max(1, FMath::CeilToInt(BreathDuration / BreathTickInterval));
	RemainingBreathTicks = TotalBreathTicks;
	BreathDamagePerTick = SkillDamage / static_cast<float>(TotalBreathTicks);

	ApplyBreathDamageTick();
	GetWorldTimerManager().SetTimer(BreathTimerHandle, this, &ADragonKnight::ApplyBreathDamageTick, BreathTickInterval, true);
}

void ADragonKnight::ApplyBreathDamageTick()
{
	if (RemainingBreathTicks <= 0)
	{
		GetWorldTimerManager().ClearTimer(BreathTimerHandle);
		return;
	}

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		--RemainingBreathTicks;
		return;
	}

	const float RangeValue = AbilitySystemComponent
		? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute())
		: DefaultRange;

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FVector MyLocation = GetActorLocation();
	const float RangeSq = FMath::Square(RangeValue);

	FMassExecutionContext ExecContext(EntityManager, 0.0f);
	TargetQueryPtr->ForEachEntityChunk(ExecContext, [this, MyLocation, RangeSq](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();
			TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
			TArrayView<FMonsterStatusFragment> Statuses = Context.GetMutableFragmentView<FMonsterStatusFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();
				if (FVector::DistSquared(MyLocation, EnemyLoc) <= RangeSq)
				{
					Statuses[i].PendingAoEDamage += BreathDamagePerTick;
				}
			}
		});

	--RemainingBreathTicks;
}

