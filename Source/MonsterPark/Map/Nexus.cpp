// Fill out your copyright notice in the Description page of Project Settings.

#include "Nexus.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MonsterPark/PlaySubSystem.h"
#include "Kismet/GameplayStatics.h"
#include "MonsterPark/BasicGameMode.h"

ANexusActor::ANexusActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
}

void ANexusActor::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UPlaySubSystem* PlaySubsystem = World->GetSubsystem<UPlaySubSystem>())
		{
			PlaySubsystem->Nexus = this;
		}
	}
}

void ANexusActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UPlaySubSystem* PlaySubsystem = World->GetSubsystem<UPlaySubSystem>())
		{
			if (PlaySubsystem->Nexus == this)
			{
				PlaySubsystem->Nexus = nullptr;
			}
		}
	}
	Super::EndPlay(EndPlayReason);
}

void ANexusActor::TakeMonsterDamage(float DamageAmount, FVector AttackerLocation)
{
	Health -= DamageAmount;

	if (Health <= 0.0f)
	{
		HandleNexusDestruction();
	}
}

FVector ANexusActor::GetTargetLocation(FVector AttackerLocation)
{
	return FVector();
}

void ANexusActor::HandleNexusDestruction()
{
	ABasicGameMode* GM = Cast<ABasicGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		GM->GameOver(false); // 패배했다는 인자를 보냄
	}
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}
