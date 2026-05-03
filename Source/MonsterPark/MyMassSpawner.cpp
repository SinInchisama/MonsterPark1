// Fill out your copyright notice in the Description page of Project Settings.


#include "MyMassSpawner.h"
#include "MassCommonTypes.h"
#include "MassSpawnLocationProcessor.h"
#include "MassEntitySubsystem.h"

void AMyMassSpawner::SpawnEntityByIndex(int32 Index, int32 Amount)
{
	if (!EntityTypes.IsValidIndex(Index)) return;

	if (SpawnDataGenerators.Num() == 0)
	{
		return;
	}

	FMassEntitySpawnDataGeneratorResult CustomResult;
	CustomResult.EntityConfigIndex = Index;
	CustomResult.NumEntities = Amount;

	CustomResult.SpawnDataProcessor = UMassSpawnLocationProcessor::StaticClass();

	FTransform SpawnerTransform = GetActorTransform();

	FMassTransformsSpawnData TransformData;
	for (int32 i = 0; i < Amount; ++i)
	{
		TransformData.Transforms.Add(SpawnerTransform);
	}
	CustomResult.SpawnData.InitializeAs<FMassTransformsSpawnData>(TransformData);
	ENetMode NetMode = GetWorld()->GetNetMode();
	if (NetMode == NM_Client)
	{
		UE_LOG(LogTemp, Error, TEXT("!!! [Client] Spawner is RUNNING. Spawning entities locally. This is likely causing duplicates."));
	}
	else if (NetMode == NM_DedicatedServer || NetMode == NM_ListenServer)
	{
		UE_LOG(LogTemp, Log, TEXT("[Server] Spawner is RUNNING. Spawning entities on server."));
	}

	SpawnGeneratedEntities({ CustomResult });
}

int32 AMyMassSpawner::GetAliveCount() const
{
	UWorld* World = GetWorld();

	UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!EntitySubsystem) return 0;

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	int32 TotalAlive = 0;

	for (const FSpawnedEntities& SpawnedGroup : AllSpawnedEntities)
	{
		for (const FMassEntityHandle& Handle : SpawnedGroup.Entities)
		{
			if (EntityManager.IsEntityValid(Handle))
			{
				TotalAlive++;
			}
		}
	}
	return TotalAlive;
}
