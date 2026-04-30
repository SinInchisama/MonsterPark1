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

	MainSpawner = Cast<AMyMassSpawner>(UGameplayStatics::GetActorOfClass(&InWorld, AMyMassSpawner::StaticClass()));
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
                OldCell->MonsterInCell.RemoveSingleSwap(Entity);
            }
        }

        SpatialGrid.FindOrAdd(NewKey).MonsterInCell.AddUnique(Entity);

        InOutLastKey = NewKey;
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

void UPlaySubSystem::StartRound(int Round,int Scale)
{
	if (!MainSpawner) return;

	MainSpawner->SpawnEntityByIndex(CurrentRound ,Scale);

}

void UPlaySubSystem::EndRound()
{
	if (!MainSpawner) return;

	int32 RemainingMonsters = MainSpawner->GetAliveCount();
    if (RemainingMonsters > 0)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            AMyBasicCharacter* PlayerChar = Cast<AMyBasicCharacter>(PC->GetPawn());
            if (PlayerChar)
            {
                PlayerChar->Miu_PlayerLife(RemainingMonsters);
				//PlayerChar->Set_PlayerMoney(20);
            }
        }
    }

    ++CurrentRound;

    OnRoundChanged.Broadcast(CurrentRound);

	MainSpawner->DoDespawning();

}