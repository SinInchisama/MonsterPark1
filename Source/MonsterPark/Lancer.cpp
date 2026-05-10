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
	Super::FindEnemiesInArea();

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem || !AbilitySystemComponent)
	{
		return;
	}

	const float RangeValue = AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute());
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
				FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();
				if (FVector::DistSquared(MyLocation, EnemyLoc) <= RadiusSq)
				{
					Statuses[i].PendingBleedDamage += Statuses[i].MaxHealt * BleedPercent;
				}
			}
		});
}

UAnimMontage* ALancer::GetDetectedMontage() const
{
	return LancerFullMontage;
}

void ALancer::PlayDetectedMontageIfNeeded()
{
	if (!bEnemyDetected)
	{
		return;
	}

	if (USkeletalMeshComponent* CharacterMesh = FindComponentByClass<USkeletalMeshComponent>())
	{
		if (UAnimInstance* AnimInst = CharacterMesh->GetAnimInstance())
		{
			if (LancerFullMontage && !AnimInst->Montage_IsPlaying(LancerFullMontage))
			{
				AnimInst->Montage_Play(LancerFullMontage);

				if (!bHasPlayedPassive)
				{
					AnimInst->Montage_JumpToSection(FName("Passive"), LancerFullMontage);
					bHasPlayedPassive = true;
				}
				else
				{
					AnimInst->Montage_JumpToSection(FName("Attack"), LancerFullMontage);
				}
			}
		}
	}
}
