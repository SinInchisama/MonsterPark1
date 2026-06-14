// Fill out your copyright notice in the Description page of Project Settings.


#include "WallActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "MonsterPark/PlaySubSystem.h"

// Sets default values
AWallActor::AWallActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	RootComponent = MeshComponent;

	AttackZone = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackZone"));
	AttackZone->SetupAttachment(RootComponent);

	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
}

// Called when the game starts or when spawned
void AWallActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (UWorld* World = GetWorld())
	{
		if (UPlaySubSystem* PlaySubsystem = World->GetSubsystem<UPlaySubSystem>())
		{
			PlaySubsystem->ActiveWalls.Add(this);
		}
	}
}

void AWallActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UPlaySubSystem* PlaySubsystem = World->GetSubsystem<UPlaySubSystem>())
		{
			PlaySubsystem->ActiveWalls.RemoveSwap(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AWallActor::TakeMonsterDamage(float DamageAmount, FVector AttackerLocation)
{
	Health -= DamageAmount;

	if (Health <= 0.f)
	{
		FVector ExplosionLocation = AttackZone ? AttackZone->GetComponentLocation() : GetActorLocation();

		if (DestructionEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				DestructionEffect,
				ExplosionLocation, 
				FRotator::ZeroRotator
			);
		}
		Destroy();
	}
}

FVector AWallActor::GetTargetLocation(FVector AttackerLocation)
{
	if (AttackZone)
	{
		FVector ClosestPoint;
		AttackZone->GetClosestPointOnCollision(AttackerLocation, ClosestPoint);
		return ClosestPoint;
	}

	return GetActorLocation();
}

