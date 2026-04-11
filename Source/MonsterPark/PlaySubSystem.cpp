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