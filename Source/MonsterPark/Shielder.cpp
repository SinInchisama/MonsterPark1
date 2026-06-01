// Fill out your copyright notice in the Description page of Project Settings.


#include "Shielder.h"
#include "MonsterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PlaySubSystem.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassEntityManager.h"
#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterConditionFragment.h"
#include "MonsterPark/Monster/Tag/KilledTag.h"
#include "MonsterPark/Monster/Tag/MonsterDyingTag.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

AShielder::AShielder()
{
    AuraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AuraComponent"));
    if (AuraComponent)
    {
        AuraComponent->SetupAttachment(GetRootComponent());
        AuraComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        AuraComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
        AuraComponent->bAutoActivate = false; 

        AuraComponent->SetRelativeLocation(FVector::ZeroVector);
    }
}

void AShielder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AShielder::BeginPlay()
{
    Super::BeginPlay();

    if (AuraComponent && AuraTemplate)
    {
        AuraComponent->SetAsset(AuraTemplate);
        AuraComponent->Activate(true);
    }
}


UAnimMontage* AShielder::GetDetectedMontage() const
{
    return ShielderFullMontage;
}

void AShielder::FindEnemiesInArea()
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem || !AbilitySystemComponent || !PlaySubsystem) return;

	const float RangeValue = AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute());
	const float AttackPowerValue = AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetAttackPowerAttribute());

	const float AttackRadiusSq = FMath::Square(RangeValue);
	const float AuraRadiusSq = FMath::Square(500.0f);
	FVector MyLocation = GetActorLocation();

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();

	for (int32 i = SlowedMonsters.Num() - 1; i >= 0; --i)
	{
		FMassEntityHandle Handle = SlowedMonsters[i];

		if (!EntityManager.IsEntityValid(Handle))
		{
			SlowedMonsters.RemoveAtSwap(i);
			continue;
		}

		FMassEntityView View(EntityManager, Handle);
		if (View.HasTag<FKilledTag>() || View.HasTag<FMonsterDyingTag>())
		{
			SlowedMonsters.RemoveAtSwap(i);
			continue;
		}

		if (const FTransformFragment* Transform = View.GetFragmentDataPtr<FTransformFragment>())
		{
			if (FVector::DistSquared(MyLocation, Transform->GetTransform().GetLocation()) > AuraRadiusSq)
			{
				if (FMonsterStatusFragment* Status = View.GetFragmentDataPtr<FMonsterStatusFragment>())
				{
					if (Status->SpeedMultiplier == SlowMultiplier)
					{
						Status->SpeedMultiplier = 400.0f;
					}
				}
				SlowedMonsters.RemoveAtSwap(i);
			}
		}
	}

	FVector TargetLocation = FVector::ZeroVector;
	FMassEntityHandle TargetHandle;
	bool bFoundAttackTarget = false;

	if (!bIsOutsideWall)
	{
		FMassExecutionContext ExecContext(EntityManager, 0.0f);
		EnemyQuery.ForEachEntityChunk(ExecContext, [this, MyLocation, AttackRadiusSq, AuraRadiusSq, &TargetLocation, &TargetHandle, &bFoundAttackTarget](FMassExecutionContext& Context)
			{
				const int32 NumEntities = Context.GetNumEntities();
				auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();
				auto Statuses = Context.GetMutableFragmentView<FMonsterStatusFragment>();

				for (int32 i = 0; i < NumEntities; ++i)
				{
					FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();
					float DistSq = FVector::DistSquared(MyLocation, EnemyLoc);

					if (DistSq <= AuraRadiusSq)
					{
						if (Statuses[i].SpeedMultiplier != SlowMultiplier)
						{
							Statuses[i].SpeedMultiplier = SlowMultiplier;
							SlowedMonsters.AddUnique(Context.GetEntity(i));
						}
					}

					if (!bFoundAttackTarget && DistSq <= AttackRadiusSq)
					{
						TargetHandle = Context.GetEntity(i);
						TargetLocation = EnemyLoc;
						bFoundAttackTarget = true;
					}
				}
			});
	}
	else
	{
		int64 MyKey = PlaySubsystem->GetGridKey(MyLocation);
		int32 CenterX = (int32)(MyKey >> 32);
		int32 CenterY = (int32)(MyKey & 0xFFFFFFFF);

		for (int32 x = -1; x <= 1; ++x)
		{
			for (int32 y = -1; y <= 1; ++y)
			{
				int64 CheckKey = ((int64)(CenterX + x) << 32) | (uint32)(CenterY + y);
				if (FGridData* Cell = PlaySubsystem->SpatialGrid.Find(CheckKey))
				{
					for (const FMonsterGridInfo& MInfo : Cell->MonsterInfos)
					{
						if (!EntityManager.IsEntityValid(MInfo.MonsterHandle)) continue;

						FMassEntityView EntityView(EntityManager, MInfo.MonsterHandle);
						if (EntityView.HasTag<FKilledTag>() || EntityView.HasTag<FMonsterDyingTag>()) continue;

						float DistSq = FVector::DistSquared(MyLocation, MInfo.Location);

						if (DistSq <= AuraRadiusSq)
						{
							if (FMonsterStatusFragment* Status = EntityView.GetFragmentDataPtr<FMonsterStatusFragment>())
							{
								if (Status->SpeedMultiplier != SlowMultiplier)
								{
									Status->SpeedMultiplier = SlowMultiplier;
									SlowedMonsters.AddUnique(MInfo.MonsterHandle);
								}
							}
						}

						if (!bFoundAttackTarget && DistSq <= AttackRadiusSq)
						{
							TargetHandle = MInfo.MonsterHandle;
							TargetLocation = MInfo.Location;
							bFoundAttackTarget = true;
						}
					}
				}
			}
		}
	}

	if (bFoundAttackTarget && TargetHandle.IsValid())
	{
		FMassEntityView TargetView(EntityManager, TargetHandle);
		if (FMonsterConditionFragment* Condition = TargetView.GetFragmentDataPtr<FMonsterConditionFragment>())
		{
			Condition->Damage += AttackPowerValue;
			bEnemyDetected = true;
			Attacking = false;

			FVector Direction = (TargetLocation - MyLocation).GetSafeNormal2D();
			if (!Direction.IsNearlyZero())
			{
				SetActorRotation(Direction.Rotation());
			}
		}
	}
}

void AShielder::PlayDetectedMontageIfNeeded()
{
    if (!bEnemyDetected) return;

    if (USkeletalMeshComponent* CharacterMesh = FindComponentByClass<USkeletalMeshComponent>())
    {
        if (UAnimInstance* AnimInst = CharacterMesh->GetAnimInstance())
        {
            if (ShielderFullMontage && !AnimInst->Montage_IsPlaying(ShielderFullMontage))
            {
                AnimInst->Montage_Play(ShielderFullMontage);

                if (!bHasPlayedPassive)
                {
  
                    AnimInst->Montage_JumpToSection(FName("Passive"), ShielderFullMontage);
                    bHasPlayedPassive = true; 
                }
                else
                {
                    AnimInst->Montage_JumpToSection(FName("Attack"), ShielderFullMontage);
                }
            }
        }
    }
}