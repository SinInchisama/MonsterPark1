// Fill out your copyright notice in the Description page of Project Settings.


#include "ExternalSpawner.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "TimerManager.h"
#include "Engine/World.h"

AExternalSpawner::AExternalSpawner()
{
    bAutoSpawnOnBeginPlay = false;

    // 틱 활성화되지만, 처음에는 꺼둡니다.
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    PrimaryActorTick.TickGroup = TG_PostUpdateWork;

    MaxEntitiesPerTick = 2; // 한 프레임에 딱 2마리만!
}

void AExternalSpawner::BeginPlay()
{
    Super::BeginPlay();

    // 서버 체크 로직 삭제!
    // 레벨이 열리고 1초 뒤에 바로 우리가 만든 커스텀 분할 스폰을 시작합니다.
    FTimerHandle StartDelayHandle;
    GetWorldTimerManager().SetTimer(StartDelayHandle, this, &AExternalSpawner::StartBatchSpawning, 1.0f, false);
}

void AExternalSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (PendingResults.Num() == 0)
    {
        SetActorTickEnabled(false); // 스폰이 다 끝나면 틱을 끕니다.
        OnSpawningFinishedEvent.Broadcast();
        return;
    }

    UMassSpawnerSubsystem* SpawnerSystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(GetWorld());
    if (!SpawnerSystem) return;

    FMassEntitySpawnDataGeneratorResult& CurrentResult = PendingResults[0];
    int32 ToSpawnNow = FMath::Min(CurrentResult.NumEntities, MaxEntitiesPerTick); // 프레임당 2마리씩

    if (ToSpawnNow > 0)
    {
        const FMassSpawnedEntityType& EntityType = EntityTypes[CurrentResult.EntityConfigIndex];
        if (const UMassEntityConfigAsset* EntityConfig = EntityType.GetEntityConfig())
        {
            const FMassEntityTemplate& EntityTemplate = EntityConfig->GetOrCreateEntityTemplate(*GetWorld());
            if (EntityTemplate.IsValid())
            {
                FSpawnedEntities& SpawnedRecord = AllSpawnedEntities.AddDefaulted_GetRef();
                SpawnedRecord.TemplateID = EntityTemplate.GetTemplateID();
                SpawnerSystem->SpawnEntities(EntityTemplate.GetTemplateID(), ToSpawnNow, CurrentResult.SpawnData, CurrentResult.SpawnDataProcessor, SpawnedRecord.Entities);
            }
        }
        CurrentResult.NumEntities -= ToSpawnNow;
    }

    if (CurrentResult.NumEntities <= 0)
    {
        PendingResults.RemoveAt(0);
    }
}

void AExternalSpawner::StartBatchSpawning()
{
    if (SpawnDataGenerators.Num() == 0 || EntityTypes.Num() == 0) return;

    AllGeneratedResults.Reset();
    float TotalProportion = 0.0f;
    for (FMassSpawnDataGenerator& Generator : SpawnDataGenerators)
    {
        if (Generator.GeneratorInstance)
        {
            Generator.bDataGenerated = false;
            TotalProportion += Generator.Proportion;
        }
    }

    const int32 TotalSpawnCount = GetSpawnCount();
    int32 SpawnCountRemaining = TotalSpawnCount;
    float ProportionRemaining = TotalProportion;

    for (FMassSpawnDataGenerator& Generator : SpawnDataGenerators)
    {
        if (Generator.GeneratorInstance && ProportionRemaining > 0.f)
        {
            const float ProportionRatio = FMath::Min(Generator.Proportion / ProportionRemaining, 1.0f);
            const int32 SpawnCount = FMath::CeilToInt(static_cast<float>(SpawnCountRemaining) * ProportionRatio);

            // 데이터 생성이 끝나면 우리의 커스텀 함수(CustomOnSpawnDataGenerationFinished)를 부르도록 연결
            FFinishedGeneratingSpawnDataSignature Delegate = FFinishedGeneratingSpawnDataSignature::CreateUObject(this, &AExternalSpawner::CustomOnSpawnDataGenerationFinished, &Generator);
            Generator.GeneratorInstance->Generate(*this, EntityTypes, SpawnCount, Delegate);

            SpawnCountRemaining -= SpawnCount;
            ProportionRemaining -= Generator.Proportion;
        }
    }
}

void AExternalSpawner::CustomOnSpawnDataGenerationFinished(TConstArrayView<FMassEntitySpawnDataGeneratorResult> Results, FMassSpawnDataGenerator* FinishedGenerator)
{
    AllGeneratedResults.Append(Results.GetData(), Results.Num());

    bool bAllDone = true;
    for (FMassSpawnDataGenerator& Generator : SpawnDataGenerators)
    {
        if (&Generator == FinishedGenerator) Generator.bDataGenerated = true;
        bAllDone &= Generator.bDataGenerated;
    }

    // 모든 위치 계산이 끝났다면
    if (bAllDone)
    {
        PendingResults.Append(AllGeneratedResults.GetData(), AllGeneratedResults.Num());
        AllGeneratedResults.Reset();

        // 타이머 대신 틱(Tick)을 켜서 매 프레임마다 스폰되게 합니다.
        SetActorTickEnabled(true);
    }
}

void AExternalSpawner::ProcessNextBatch()
{
    // 대기열에 남은 게 없으면 스폰 종료
    if (PendingResults.Num() == 0)
    {
        GetWorldTimerManager().ClearTimer(BatchSpawnTimerHandle);
        OnSpawningFinishedEvent.Broadcast(); // 게임 로직(웨이브 시작 등)에 스폰 끝났다고 알림
        return;
    }

    UMassSpawnerSubsystem* SpawnerSystem = UWorld::GetSubsystem<UMassSpawnerSubsystem>(GetWorld());
    if (!SpawnerSystem) return;

    FMassEntitySpawnDataGeneratorResult& CurrentResult = PendingResults[0];

    // 이번 프레임에 스폰할 양 (MaxEntitiesPerBatch 만큼 끊어서)
    int32 ToSpawnNow = FMath::Min(CurrentResult.NumEntities, MaxEntitiesPerBatch);

    if (ToSpawnNow > 0)
    {
        const FMassSpawnedEntityType& EntityType = EntityTypes[CurrentResult.EntityConfigIndex];
        if (const UMassEntityConfigAsset* EntityConfig = EntityType.GetEntityConfig())
        {
            const FMassEntityTemplate& EntityTemplate = EntityConfig->GetOrCreateEntityTemplate(*GetWorld());
            if (EntityTemplate.IsValid())
            {
                FSpawnedEntities& SpawnedRecord = AllSpawnedEntities.AddDefaulted_GetRef();
                SpawnedRecord.TemplateID = EntityTemplate.GetTemplateID();

                // 실제 스폰 진행
                SpawnerSystem->SpawnEntities(EntityTemplate.GetTemplateID(), ToSpawnNow, CurrentResult.SpawnData, CurrentResult.SpawnDataProcessor, SpawnedRecord.Entities);
            }
        }
        CurrentResult.NumEntities -= ToSpawnNow;
    }

    // 이번 배치를 다 썼으면 대기열에서 삭제 (다음 틱엔 다음 배치를 처리)
    if (CurrentResult.NumEntities <= 0)
    {
        PendingResults.RemoveAt(0);
    }
}