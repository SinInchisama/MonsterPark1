// Fill out your copyright notice in the Description page of Project Settings.


#include "Wizard.h"

#include "Engine/World.h"
#include "TimerManager.h"

#include "PlaySubSystem.h"
#include "MonsterAttributeSet.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "MassEntitySubsystem.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassEntityView.h"
#include "MassCommonFragments.h" 

#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterConditionFragment.h"
#include "MonsterPark/Monster/Tag/MonsterDyingTag.h"
#include "MonsterPark/Monster/Tag/KilledTag.h"

void AWizard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
};

void AWizard::FindEnemiesInArea()
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
				if (bFoundTarget) return;

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

								FMassEntityView EntityView(EntityManager, MInfo.MonsterHandle);
								if (EntityView.HasTag<FKilledTag>() || EntityView.HasTag<FMonsterDyingTag>()) continue;

								if (FMonsterStatusFragment* Status = EntityView.GetFragmentDataPtr<FMonsterStatusFragment>())
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
		if (FirePillarTemplate)
		{
			UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), FirePillarTemplate, TargetLocation, FRotator::ZeroRotator, FVector(1.0f)
			);

			if (SpawnedEffect)
			{
				TWeakObjectPtr<UNiagaraComponent> WeakEffect = SpawnedEffect;
				FTimerHandle EffectTimerHandle;
				GetWorld()->GetTimerManager().SetTimer(EffectTimerHandle, [WeakEffect]()
					{
						if (WeakEffect.IsValid())
						{
							WeakEffect->Deactivate();
							WeakEffect->DestroyComponent();
						}
					}, 0.3f, false);
			}
		}

		bEnemyDetected = true;
		Attacking = false;

		FVector Direction = (TargetLocation - MyLocation).GetSafeNormal2D();
		if (!Direction.IsNearlyZero())
		{
			SetActorRotation(Direction.Rotation());
		}
	}
}

UAnimMontage* AWizard::GetDetectedMontage() const
{
    return WizardFullMontage;
}

void AWizard::PlayDetectedMontageIfNeeded()
{
    if (!bEnemyDetected) return;

    USkeletalMeshComponent* CharacterMesh = GetMesh();
    if (CharacterMesh)
    {
        if (UAnimInstance* AnimInst = CharacterMesh->GetAnimInstance())
        {
            if (WizardFullMontage && !AnimInst->Montage_IsPlaying(WizardFullMontage))
            {
                AnimInst->Montage_Play(WizardFullMontage);

                if (!bHasPlayedPassive)
                {
                    AnimInst->Montage_JumpToSection(FName("Passive"), WizardFullMontage);
                    bHasPlayedPassive = true;
                }
                else
                {
                    AnimInst->Montage_JumpToSection(FName("Attack"), WizardFullMontage);
                }
            }
        }
    }
}