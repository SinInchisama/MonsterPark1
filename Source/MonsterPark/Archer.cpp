// Fill out your copyright notice in the Description page of Project Settings.


#include "Archer.h"
#include "MonsterPark/Hero/ArrowProjectile.h"
#include "MassEntitySubsystem.h"
#include "MonsterAttributeSet.h"
#include "PlaySubSystem.h"

#include "MassCommonFragments.h"
#include "MonsterPark/Monster/Fragment/FMonsterConditionFragment.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassEntityView.h"

void AArcher::BeginPlay()
{
	Super::BeginPlay();

	if (ArrowClass && GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		for (int32 i = 0; i < ArrowPoolSize; ++i)
		{
			AArrowProjectile* NewArrow = GetWorld()->SpawnActor<AArrowProjectile>(ArrowClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (NewArrow)
			{
				NewArrow->DeactivateProjectile();
				ArrowPool.Add(NewArrow);
			}
		}
	}
}

void AArcher::FindEnemiesInArea()
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem) return;

	const float RangeValue = AbilitySystemComponent
		? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute())
		: DefaultRange;

	const float AttackPowerValue = AbilitySystemComponent
		? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetAttackPowerAttribute())
		: DefaultAttackPower;

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	FVector MyLocation = GetActorLocation();
	float RadiusSq = FMath::Square(RangeValue);

	LoadedArrows.Empty();
	FMassEntityHandle TargetHandle;
	bool bFound = false;
	FVector TargetLoc = FVector::ZeroVector;

	if (!bIsOutsideWall)
	{
		FMassExecutionContext ExecContext(EntityManager, 0.0f);
		TargetQueryPtr->ForEachEntityChunk(ExecContext, [this, MyLocation, RadiusSq, &bFound, &TargetHandle, &TargetLoc](FMassExecutionContext& Context)
			{
				if (bFound) return;
				const int32 NumEntities = Context.GetNumEntities();
				TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();

				for (int32 i = 0; i < NumEntities; ++i)
				{
					FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();
					if (FVector::DistSquared(MyLocation, EnemyLoc) <= RadiusSq)
					{
						TargetHandle = Context.GetEntity(i);
						TargetLoc = EnemyLoc;
						bFound = true;
						break;
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
			if (bFound) break;
			for (int32 y = -1; y <= 1; ++y)
			{
				int64 CheckKey = ((int64)(CenterX + x) << 32) | (uint32)(CenterY + y);
				if (FGridData* Cell = PlaySubsystem->SpatialGrid.Find(CheckKey))
				{
					for (const FMonsterGridInfo& MInfo : Cell->MonsterInfos)
					{
						if (FVector::DistSquared(MyLocation, MInfo.Location) <= RadiusSq)
						{
							if (!EntityManager.IsEntityValid(MInfo.MonsterHandle)) continue;

							FMassEntityView EntityView(EntityManager, MInfo.MonsterHandle);
							if (EntityView.GetFragmentDataPtr<FMonsterConditionFragment>())
							{
								TargetHandle = MInfo.MonsterHandle;
								TargetLoc = MInfo.Location;
								bFound = true;
								break;
							}
						}
					}
				}
				if (bFound) break;
			}
		}
	}

	if (bFound && TargetHandle.IsValid())
	{
		bEnemyDetected = true;
		Attacking = false;

		// 회전 처리
		FVector Direction = (TargetLoc - MyLocation).GetSafeNormal2D();
		if (!Direction.IsNearlyZero())
		{
			SetActorRotation(Direction.Rotation());
		}

		AArrowProjectile* ArrowToLoad = nullptr;
		for (AArrowProjectile* Arrow : ArrowPool)
		{
			if (IsValid(Arrow) && !Arrow->IsArrowActive() && !LoadedArrows.Contains(Arrow))
			{
				ArrowToLoad = Arrow;
				break;
			}
		}

		if (!ArrowToLoad && ArrowClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			ArrowToLoad = GetWorld()->SpawnActor<AArrowProjectile>(ArrowClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (ArrowToLoad) ArrowPool.Add(ArrowToLoad);
		}

		if (ArrowToLoad)
		{
			FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.0f + FVector(0.f, 0.f, 50.f);
			FRotator SpawnRotation = GetActorRotation();

			ArrowToLoad->ActivateProjectile(SpawnLocation, SpawnRotation, TargetHandle, AttackPowerValue);
			ArrowToLoad->SetActorTickEnabled(false);

			ArrowToLoad->SetActorHiddenInGame(true);

			LoadedArrows.Add(ArrowToLoad);

			GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AArcher::ExecuteDelayedFire, ArrowFireDelay, false);
		}
	}
}

void AArcher::ExecuteDelayedFire()
{
	for (AArrowProjectile* Arrow : LoadedArrows)
	{
		if (IsValid(Arrow) && Arrow->IsArrowActive())
		{
			FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.0f + FVector(0.f, 0.f, 50.f);
			Arrow->SetActorLocation(SpawnLocation);
			Arrow->SetActorRotation(GetActorRotation());

			Arrow->SetActorHiddenInGame(false);
			Arrow->SetActorTickEnabled(true);
		}
	}

	LoadedArrows.Empty();
}