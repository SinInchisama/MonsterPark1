// Fill out your copyright notice in the Description page of Project Settings.


#include "MyMassSpawner.h"
#include "MassCommonTypes.h"
#include "MassSpawnLocationProcessor.h"

void AMyMassSpawner::SpawnEntityByIndex(int32 Index, int32 Amount)
{
	if (!EntityTypes.IsValidIndex(Index)) return;

	if (SpawnDataGenerators.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnDataGenerators is empty! Please add at least one Generator in Blueprints."));
		return;
	}

	FMassEntitySpawnDataGeneratorResult CustomResult;
	CustomResult.EntityConfigIndex = Index;
	CustomResult.NumEntities = Amount;

	CustomResult.SpawnDataProcessor = UMassSpawnLocationProcessor::StaticClass();

	FTransform SpawnerTransform = GetActorTransform();

	// 위치 데이터 설정
	FMassTransformsSpawnData TransformData;
	for (int32 i = 0; i < Amount; ++i)
	{
		TransformData.Transforms.Add(SpawnerTransform);
	}
	CustomResult.SpawnData.InitializeAs<FMassTransformsSpawnData>(TransformData);



	SpawnGeneratedEntities({ CustomResult });
}
