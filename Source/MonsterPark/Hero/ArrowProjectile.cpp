// Fill out your copyright notice in the Description page of Project Settings.


#include "ArrowProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MonsterPark/Monster/Fragment/FMonsterConditionFragment.h"

// Sets default values
AArrowProjectile::AArrowProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	DeactivateProjectile();
}

// Called when the game starts or when spawned
void AArrowProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
}

void AArrowProjectile::ActivateProjectile(FVector InLocation, FRotator InRotation, FMassEntityHandle InTarget, float InDamage)
{
	SetActorLocation(InLocation);
	SetActorRotation(InRotation);
	TargetEntity = InTarget;
	DamageToApply = InDamage;

	bIsActive = true;

	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);

	Velocity = InRotation.Vector() * ArrowSpeed;
}

void AArrowProjectile::DeactivateProjectile()
{
	bIsActive = false;

	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);

	Velocity = FVector::ZeroVector;
}

void AArrowProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsActive || !MassEntitySubsystem) return;

	FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();

	if (!EntityManager.IsEntityValid(TargetEntity))
	{
		DeactivateProjectile();
		return;
	}

	if (FTransformFragment* TargetTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(TargetEntity))
	{
		FVector TargetLocation = TargetTransform->GetTransform().GetLocation();
		TargetLocation.Z += 50.0f; 

		FVector MyLocation = GetActorLocation();


		float DistSq = FVector::DistSquared(MyLocation, TargetLocation);

		if (DistSq <= FMath::Square(60.0f))
		{
			if (FMonsterConditionFragment* Condition = EntityManager.GetFragmentDataPtr<FMonsterConditionFragment>(TargetEntity))
			{
				Condition->Damage += DamageToApply;
			}

			DeactivateProjectile();
			return;
		}

		FVector NewDirection = (TargetLocation - MyLocation).GetSafeNormal();
		Velocity = NewDirection * ArrowSpeed;

		AddActorWorldOffset(Velocity * DeltaTime);
		SetActorRotation(Velocity.Rotation());
	}
}

