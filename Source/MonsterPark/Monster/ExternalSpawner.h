// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSpawner.h"
#include "ExternalSpawner.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API AExternalSpawner : public AMassSpawner
{
	GENERATED_BODY()
	

public:
    AExternalSpawner();

protected:
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

    void StartBatchSpawning();
    void CustomOnSpawnDataGenerationFinished(TConstArrayView<FMassEntitySpawnDataGeneratorResult> Results, FMassSpawnDataGenerator* FinishedGenerator);

    // 타이머가 주기적으로 호출할 실제 분할 스폰 로직
    void ProcessNextBatch();

    // --- 분할 생성 설정 ---
    UPROPERTY(EditAnywhere, Category = "Mass|Batch")
    int32 MaxEntitiesPerBatch = 100; // 프레임당 스폰 최대 개수 (추천: 100~150)

    UPROPERTY(EditAnywhere, Category = "Mass|Batch")
    float BatchInterval = 0.1f;      // 스폰 간격 (0.1초)

    // 내부 관리용 변수
    TArray<FMassEntitySpawnDataGeneratorResult> PendingResults;
    FTimerHandle BatchSpawnTimerHandle;

    UPROPERTY(EditAnywhere, Category = "Mass|Batch")
    int32 MaxEntitiesPerTick = 2;
};
