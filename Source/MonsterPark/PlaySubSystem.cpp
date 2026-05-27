// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaySubSystem.h"
#include "MyBasicCharacter.h"
#include "Map/WallActor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Algo/Reverse.h"

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

FVector UPlaySubSystem::GridToPos(const FIntVector& GridKey) const
{
    return FVector(
        (GridKey.X + 0.5f) * 100.f,
        (GridKey.Y + 0.5f) * 100.f,
        0.f 
    );
}

TArray<FIntVector> UPlaySubSystem::FindPath(const FIntVector& StartGrid, const FIntVector& TargetGrid)
{
    TArray<FIntVector> Path;

    if (IsGridBlocked(TargetGrid) || IsGridBlocked(StartGrid) || StartGrid == TargetGrid)
    {
        return Path;
    }

    TArray<FIntVector> OpenList;          
    TSet<FIntVector> ClosedSet;           
    TMap<FIntVector, FIntVector> ParentMap; 
    TMap<FIntVector, int32> GCostMap;     
    TMap<FIntVector, int32> FCostMap;     

    OpenList.Add(StartGrid);
    GCostMap.Add(StartGrid, 0);
    FCostMap.Add(StartGrid, 0); 

    while (OpenList.Num() > 0)
    {
        int32 LowestIndex = 0;
        int32 LowestFCost = FCostMap[OpenList[0]];

        for (int32 i = 1; i < OpenList.Num(); ++i)
        {
            int32 Cost = FCostMap[OpenList[i]];
            if (Cost < LowestFCost)
            {
                LowestFCost = Cost;
                LowestIndex = i;
            }
        }

        FIntVector Current = OpenList[LowestIndex];
        OpenList.RemoveAt(LowestIndex);

        if (Current == TargetGrid)
        {
            FIntVector Trace = Current;
            while (ParentMap.Contains(Trace))
            {
                Path.Add(Trace);
                Trace = ParentMap[Trace];
            }
            Algo::Reverse(Path);
            return Path;
        }

        ClosedSet.Add(Current);

        TArray<FIntVector> Directions = {
            FIntVector(1, 0, 0), FIntVector(-1, 0, 0), FIntVector(0, 1, 0), FIntVector(0, -1, 0),  
            FIntVector(1, 1, 0), FIntVector(1, -1, 0), FIntVector(-1, 1, 0), FIntVector(-1, -1, 0) 
        };

        for (const FIntVector& Dir : Directions)
        {
            FIntVector Neighbor = Current + Dir;

            if (ClosedSet.Contains(Neighbor) || IsGridBlocked(Neighbor))
            {
                continue;
            }

            int32 MoveCost = (Dir.X != 0 && Dir.Y != 0) ? 14 : 10;

            if (MoveCost == 14)
            {
                if (IsGridBlocked(FIntVector(Current.X + Dir.X, Current.Y, 0)) &&
                    IsGridBlocked(FIntVector(Current.X, Current.Y + Dir.Y, 0)))
                {
                    continue;
                }
            }

            int32 TentativeGCost = GCostMap[Current] + MoveCost;
            int32* ExistingGCost = GCostMap.Find(Neighbor);

            if (!ExistingGCost || TentativeGCost < *ExistingGCost)
            {
                ParentMap.Add(Neighbor, Current);
                GCostMap.Add(Neighbor, TentativeGCost);

                int32 dx = FMath::Abs(Neighbor.X - TargetGrid.X);
                int32 dy = FMath::Abs(Neighbor.Y - TargetGrid.Y);

                int32 HCost = 10 * (dx + dy) + (14 - 20) * FMath::Min(dx, dy);

                FCostMap.Add(Neighbor, TentativeGCost + HCost);

                if (!OpenList.Contains(Neighbor))
                {
                    OpenList.Add(Neighbor);
                }
            }
        }
    }

    return Path;
}

bool UPlaySubSystem::HasLineOfSight(const FIntVector& Start, const FIntVector& End) const
{
    int32 x1 = Start.X; int32 y1 = Start.Y;
    int32 x2 = End.X;   int32 y2 = End.Y;

    int32 dx = FMath::Abs(x2 - x1);
    int32 dy = FMath::Abs(y2 - y1);
    int32 sx = (x1 < x2) ? 1 : -1;
    int32 sy = (y1 < y2) ? 1 : -1;
    int32 err = dx - dy;

    while (true)
    {
        if (IsGridBlocked(FIntVector(x1, y1, 0)))
        {
            return false; 
        }

        if (x1 == x2 && y1 == y2) break;

        int32 e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
    return true; 
}

TArray<FIntVector> UPlaySubSystem::SmoothPath(const TArray<FIntVector>& InPath)
{
    if (InPath.Num() <= 2) return InPath;

    TArray<FIntVector> SmoothedPath;
    SmoothedPath.Add(InPath[0]); 

    int32 CurrentIndex = 0;
    while (CurrentIndex < InPath.Num() - 1)
    {
        int32 FurthestVisibleIndex = CurrentIndex + 1;

        for (int32 i = CurrentIndex + 2; i < InPath.Num(); ++i)
        {
            if (HasLineOfSight(InPath[CurrentIndex], InPath[i]))
            {
                FurthestVisibleIndex = i; 
            }
            else
            {
                break; 
            }
        }

        SmoothedPath.Add(InPath[FurthestVisibleIndex]);
        CurrentIndex = FurthestVisibleIndex;
    }

    return SmoothedPath;
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
                Spawner->SpawnEntityByIndex(CurrentRound, 10);
            }
            else
            {
                int R = CurrentRound;
                FTimerHandle TempHandle;
                TimerManager.SetTimer(TempHandle, [Spawner, R, Scale]()
                    {
                        if (IsValid(Spawner))
                        {
                            Spawner->SpawnEntityByIndex(R, 10);
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