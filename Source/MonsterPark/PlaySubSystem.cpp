// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaySubSystem.h"
#include "MyBasicCharacter.h"
#include "Map/WallActor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

void UPlaySubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPlaySubSystem::Deinitialize()
{
	Super::Deinitialize();
}

void UPlaySubSystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

    TArray<AActor*> FoundActors;

    UGameplayStatics::GetAllActorsOfClass(&InWorld, AMyMassSpawner::StaticClass(), FoundActors);

    MainSpawners.Empty(); // 기존 데이터가 있다면 비워줍니다.
    for (AActor* Actor : FoundActors)
    {
        if (AMyMassSpawner* Spawner = Cast<AMyMassSpawner>(Actor))
        {
            MainSpawners.Add(Spawner);
        }
    }
}

void UPlaySubSystem::UpdateHeroLocation(AActor* Hero, int64& InOutLastKey, FVector NewLocation)
{
 
    int64 NewKey = GetGridKey(NewLocation);

    if (InOutLastKey == NewKey) return;

    if (FGridData* OldCell = SpatialGrid.Find(InOutLastKey))
    {
        OldCell->HeroesInCell.RemoveSingleSwap(Hero);
    }

    SpatialGrid.FindOrAdd(NewKey).HeroesInCell.AddUnique(Hero);

    InOutLastKey = NewKey;
}

void UPlaySubSystem::UpdateMonsterLocation(FMassEntityHandle Entity, int64& InOutLastKey, FVector NewLocation)
{
    int64 NewKey = GetGridKey(NewLocation);

    if (InOutLastKey != NewKey)
    {
        if (InOutLastKey != -1)
        {
            if (FGridData* OldCell = SpatialGrid.Find(InOutLastKey))
            {
                OldCell->MonsterInfos.RemoveAll([Entity](const FMonsterGridInfo& Info) {
                    return Info.MonsterHandle == Entity;
                    });
            }
        }

        FMonsterGridInfo NewInfo;
        NewInfo.MonsterHandle = Entity;
        NewInfo.Location = NewLocation;
        SpatialGrid.FindOrAdd(NewKey).MonsterInfos.Add(NewInfo);

        InOutLastKey = NewKey;
    }
    else
    {
        if (FGridData* CurrentCell = SpatialGrid.Find(NewKey))
        {
            for (FMonsterGridInfo& Info : CurrentCell->MonsterInfos)
            {
                if (Info.MonsterHandle == Entity)
                {
                    Info.Location = NewLocation; 
                    break;
                }
            }
        }
    }
}

void UPlaySubSystem::RemoveHeroFromGrid(AActor* Hero, int64& InOutLastKey)
{
    if (FGridData* Cell = SpatialGrid.Find(InOutLastKey))
    {
        Cell->HeroesInCell.RemoveSingleSwap(Hero);
        if (Cell->HeroesInCell.Num() == 0)
        {
            SpatialGrid.Remove(InOutLastKey);
        }
    }
    InOutLastKey = -1;
}

AActor* UPlaySubSystem::FindNearestHeroInGrid(FVector SearchLocation, float SearchRadius)
{
    float MinDistSq = FMath::Square(SearchRadius);
    AActor* NearestHero = nullptr;
    int64 CenterKey = GetGridKey(SearchLocation);

    int32 CenterX = (int32)(CenterKey >> 32);
    int32 CenterY = (int32)(CenterKey & 0xFFFFFFFF);

    for (int32 x = -1; x <= 1; ++x)
    {
        for (int32 y = -1; y <= 1; ++y)
        {
            int64 CheckKey = ((int64)(CenterX + x) << 32) | (uint32)(CenterY + y);
            if (FGridData* Cell = SpatialGrid.Find(CheckKey))
            {
                for (AActor* Hero : Cell->HeroesInCell)
                {
                    if (Hero)
                    {
                        float DistSq = FVector::DistSquared(SearchLocation, Hero->GetActorLocation());
                        if (DistSq < MinDistSq)
                        {
                            MinDistSq = DistSq;
                            NearestHero = Hero;
                        }
                    }
                }
            }
        }
    }
    return NearestHero;
}

AActor* UPlaySubSystem::FindFinalRoundTarget(FVector MonsterLocation)
{
    if (ActiveWalls.Num() > 0)
    {
        AActor* BestTarget = nullptr;
        float MinDistSq = MAX_FLT;

        for (AActor* WallActor : ActiveWalls)
        {
            if (AWallActor* Wall = Cast<AWallActor>(WallActor))
            {
                FVector MeshLocation;
                Wall->AttackZone->GetClosestPointOnCollision(MonsterLocation, MeshLocation);

                float DistSq = FVector::DistSquared(MonsterLocation, MeshLocation);
                if (DistSq < MinDistSq)
                {
                    MinDistSq = DistSq;
                    BestTarget = Wall;
                }
            }
        }
        return BestTarget;
    }

    return Nexus;
}

void UPlaySubSystem::OnWallDestroyed(AActor* DestroyedWall)
{
    ActiveWalls.RemoveSingleSwap(DestroyedWall);
}

FIntVector UPlaySubSystem::PosToGrid(const FVector& Pos) const
{
    return FIntVector(
        FMath::FloorToInt(Pos.X / 100.f),
        FMath::FloorToInt(Pos.Y / 100.f),
        0
    );
}

bool UPlaySubSystem::IsGridBlocked(const FIntVector& GridKey) const
{
    const int32* OccupyCount = ObstacleMap.Find(GridKey);
    return OccupyCount && (*OccupyCount > 0);
}

void UPlaySubSystem::RegisterObstacle(const TArray<FIntVector>& GridKeys)
{
    for (const FIntVector& Key : GridKeys)
    {
        int32& Count = ObstacleMap.FindOrAdd(Key);
        Count++;
    }
}

void UPlaySubSystem::StartRound(int Round,int Scale)
{
    if (MainSpawners.Num() == 0) return;

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    for (int32 i = 0; i < MainSpawners.Num(); ++i)
    {
        AMyMassSpawner* Spawner = MainSpawners[i];

        if (IsValid(Spawner))
        {
            float DelayTime = i * 0.2f;

            if (DelayTime <= 0.0f)
            {
                Spawner->SpawnEntityByIndex(CurrentRound, Scale);
            }
            else
            {
                FTimerHandle TempHandle;
                TimerManager.SetTimer(TempHandle, [Spawner, Round, Scale]()
                    {
                        if (IsValid(Spawner))
                        {
                            Spawner->SpawnEntityByIndex(Round, Scale);
                        }
                    }, DelayTime, false);
            }
        }
    }

}

void UPlaySubSystem::EndRound()
{
    if (MainSpawners.Num() == 0) return;

    int32 TotalRemainingMonsters = 0;

    for (AMyMassSpawner* Spawner : MainSpawners)
    {
        if (IsValid(Spawner))
        {
            TotalRemainingMonsters += Spawner->GetAliveCount();
            Spawner->DoDespawning();
        }
    }

    if (TotalRemainingMonsters > 0)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            AMyBasicCharacter* PlayerChar = Cast<AMyBasicCharacter>(PC->GetPawn());
            if (PlayerChar)
            {
                PlayerChar->Miu_PlayerLife(TotalRemainingMonsters);
                PlayerChar->Set_PlayerMoney(20);
            }
        }
    }

    ++CurrentRound;
    OnRoundChanged.Broadcast(CurrentRound);

}