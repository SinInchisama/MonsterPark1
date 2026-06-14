// Fill out your copyright notice in the Description page of Project Settings.


#include "Wyvern.h"
#include "MonsterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PlaySubSystem.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassEntityManager.h"
#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MonsterPark/Monster/Tag/KilledTag.h"
#include "MonsterPark/Monster/Tag/MonsterDyingTag.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/ConstructorHelpers.h"

AWyvern::AWyvern()
{
	static ConstructorHelpers::FObjectFinder<UParticleSystem> WyvernBreathFX(TEXT("/Game/M5VFXVOL2/Particles/Fire_for_Dir/Fire_Dir_04.Fire_Dir_04"));
	if (WyvernBreathFX.Succeeded())
	{
		BreathTemplate = WyvernBreathFX.Object;
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> WyvernImpactFX(TEXT("/Game/M5VFXVOL2/Niagara/Explosion/NFire_Exp_00.NFire_Exp_00"));
	if (WyvernImpactFX.Succeeded())
	{
		ImpactTemplate = WyvernImpactFX.Object;
	}
}

void AWyvern::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWyvern::FindEnemiesInArea()
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
							Statuses[i].PendingBleedDamage += Statuses[i].MaxHealt * BurnPercent;
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
									Status->PendingBleedDamage += Status->MaxHealt * BurnPercent;
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

		SpawnImpactVFX(TargetLocation);
		SpawnBreathVFX();
	}
}

void AWyvern::SpawnBreathVFX()
{
	UWorld* World = GetWorld();
	if (!World || !BreathTemplate)
	{
		return;
	}

	FVector SpawnLocation = GetActorLocation()
		+ GetActorForwardVector() * BreathFallbackOffset.X
		+ GetActorRightVector() * BreathFallbackOffset.Y
		+ FVector(0.0f, 0.0f, BreathFallbackOffset.Z);
	FRotator SpawnRotation = GetActorRotation();

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		if (CharacterMesh->DoesSocketExist(BreathSocketName) || CharacterMesh->GetBoneIndex(BreathSocketName) != INDEX_NONE)
		{
			SpawnLocation = CharacterMesh->GetSocketLocation(BreathSocketName);
			SpawnRotation = CharacterMesh->GetSocketRotation(BreathSocketName);
		}
	}

	UParticleSystemComponent* Breath = UGameplayStatics::SpawnEmitterAtLocation(
		World,
		BreathTemplate,
		SpawnLocation,
		SpawnRotation,
		BreathScale,
		true
	);

	if (!Breath)
	{
		return;
	}

	Breath->SetVisibility(true, true);
	Breath->SetComponentTickEnabled(true);
	Breath->ActivateSystem(true);

	if (BreathLifeTime > 0.0f)
	{
		TWeakObjectPtr<UParticleSystemComponent> BreathPtr(Breath);
		FTimerHandle BreathTimerHandle;
		GetWorldTimerManager().SetTimer(
			BreathTimerHandle,
			[BreathPtr]()
			{
				if (BreathPtr.IsValid())
				{
					BreathPtr->DeactivateSystem();
				}
			},
			BreathLifeTime,
			false
		);
	}
}

void AWyvern::SpawnImpactVFX(const FVector& TargetLocation)
{
	if (!GetWorld() || !ImpactTemplate)
	{
		return;
	}

	UNiagaraComponent* Impact = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ImpactTemplate,
		TargetLocation,
		FRotator::ZeroRotator,
		ImpactScale,
		true
	);

	if (!Impact || ImpactLifeTime <= 0.0f)
	{
		return;
	}

	TWeakObjectPtr<UNiagaraComponent> ImpactPtr(Impact);
	FTimerHandle ImpactTimerHandle;
	GetWorldTimerManager().SetTimer(
		ImpactTimerHandle,
		[ImpactPtr]()
		{
			if (ImpactPtr.IsValid())
			{
				ImpactPtr->Deactivate();
				ImpactPtr->DestroyComponent();
			}
		},
		ImpactLifeTime,
		false
	);
}

UAnimMontage* AWyvern::GetDetectedMontage() const
{
	return WyvernFullMontage;
}

void AWyvern::PlayDetectedMontageIfNeeded()
{
	PlayDetectedMontageSection(WyvernFullMontage, bHasPlayedPassive);
}
