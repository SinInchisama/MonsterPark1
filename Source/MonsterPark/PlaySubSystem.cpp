// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaySubSystem.h"
#include "MyBasicCharacter.h"
#include "Kismet/GameplayStatics.h"

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

    // 2. 격자 칸이 바뀌었을 때만 업데이트 수행 (성능 최적화)
    if (InOutLastKey != NewKey)
    {
        // 이전 칸이 유효했다면 이전 칸에서 엔티티 제거
        if (InOutLastKey != -1)
        {
            if (FGridData* OldCell = SpatialGrid.Find(InOutLastKey))
            {
                // RemoveSingleSwap은 배열 순서를 유지하지 않지만 속도가 매우 빠름 (TArray 최적화)
                OldCell->MonsterInCell.RemoveSingleSwap(Entity);
            }
        }

        // 새 칸에 엔티티 추가
        SpatialGrid.FindOrAdd(NewKey).MonsterInCell.AddUnique(Entity);

        // 3. 레퍼런스로 넘어온 LastKey 업데이트 (프래그먼트에 저장됨)
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

void UPlaySubSystem::StartRound(int Round,int Scale)
{
	if (!MainSpawner) return;

	MainSpawner->SpawnEntityByIndex(Round,Scale);

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
				PlayerChar->Set_PlayerMoney(20);
            }
        }
    }

	MainSpawner->DoDespawning();

}