// Fill out your copyright notice in the Description page of Project Settings.

#include "SageMeteorProjectile.h"
#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

ASageMeteorProjectile::ASageMeteorProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	MeteorRock = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeteorRock"));
	RootComponent = MeteorRock;
	MeteorRock->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeteorRock->SetRelativeRotation(FRotator(-20.0f, 35.0f, 12.0f));
	MeteorRock->SetRelativeScale3D(FVector(2.5f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeteorMesh(TEXT("/Game/Map/Stone/BigRock.BigRock"));
	if (MeteorMesh.Succeeded())
	{
		MeteorRock->SetStaticMesh(MeteorMesh.Object);
	}

	MeteorTrailFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MeteorTrailFX"));
	MeteorTrailFX->SetupAttachment(RootComponent);
	MeteorTrailFX->SetRelativeLocation(FVector(-65.0f, 0.0f, 15.0f));
	MeteorTrailFX->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	MeteorTrailFX->SetRelativeScale3D(FVector(1.8f));
	MeteorTrailFX->SetAutoActivate(true);

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> MeteorTrail(TEXT("/Game/VFX/Meteor/NS_SingleMeteor.NS_SingleMeteor"));
	if (MeteorTrail.Succeeded())
	{
		MeteorTrailFX->SetAsset(MeteorTrail.Object);
	}

	MeteorGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("MeteorGlow"));
	MeteorGlow->SetupAttachment(RootComponent);
	MeteorGlow->SetIntensity(8500.0f);
	MeteorGlow->SetAttenuationRadius(850.0f);
	MeteorGlow->SetLightColor(FLinearColor(1.0f, 0.38f, 0.11f));
	MeteorGlow->SetCastShadows(false);

	DeactivateProjectile();
}

void ASageMeteorProjectile::BeginPlay()
{
	Super::BeginPlay();

	MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	DeactivateProjectile();
}

void ASageMeteorProjectile::ActivateProjectile(FVector InLocation, FRotator InRotation, FMassEntityHandle InTarget, float InDamage)
{
	SetActorLocation(InLocation);
	SetActorRotation(FRotator::ZeroRotator);

	TargetEntity = InTarget;
	ImpactLocation = InLocation - FVector(0.0f, 0.0f, MeteorStartHeight - ImpactHeight);
	DamageToApply = InDamage;
	bIsActive = true;
	FallElapsedTime = 0.0f;

	UE_LOG(LogTemp, Warning, TEXT("SageMeteor Activate: Location=%s TargetValid=%s Damage=%.1f"), *InLocation.ToString(), InTarget.IsValid() ? TEXT("true") : TEXT("false"), InDamage);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("SageMeteor Activate"));
	}

	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
	MeteorRock->SetVisibility(true, true);

	if (MeteorTrailFX)
	{
		MeteorTrailFX->SetVisibility(true, true);
		MeteorTrailFX->SetRenderingEnabled(true);
		MeteorTrailFX->ResetSystem();
		MeteorTrailFX->Activate(true);
	}

	if (MeteorGlow)
	{
		MeteorGlow->SetVisibility(true, true);
	}
}

void ASageMeteorProjectile::DeactivateProjectile()
{
	bIsActive = false;
	ImpactLocation = FVector::ZeroVector;
	FallElapsedTime = 0.0f;

	if (MeteorTrailFX)
	{
		MeteorTrailFX->Deactivate();
		MeteorTrailFX->SetVisibility(false, true);
	}

	if (MeteorGlow)
	{
		MeteorGlow->SetVisibility(false, true);
	}

	MeteorRock->SetVisibility(false, true);

	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
}

void ASageMeteorProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsActive || !MassEntitySubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = MassEntitySubsystem->GetMutableEntityManager();
	FallElapsedTime += DeltaTime;

	const float SafeFallDuration = FMath::Max(FallDuration, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(FallElapsedTime / SafeFallDuration, 0.0f, 1.0f);
	const float HeightOffset = FMath::Lerp(MeteorStartHeight, ImpactHeight, Alpha);
	const FVector MeteorLocation(ImpactLocation.X, ImpactLocation.Y, ImpactLocation.Z + HeightOffset);

	SetActorLocation(MeteorLocation);

	if (Alpha >= 1.0f)
	{
		if (EntityManager.IsEntityValid(TargetEntity))
		{
			if (FMonsterStatusFragment* Status = EntityManager.GetFragmentDataPtr<FMonsterStatusFragment>(TargetEntity))
			{
				Status->PendingAoEDamage += DamageToApply;
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("SageMeteor Impact: Location=%s Damage=%.1f"), *MeteorLocation.ToString(), DamageToApply);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("SageMeteor Impact"));
		}

		DeactivateProjectile();
		return;
	}
}
