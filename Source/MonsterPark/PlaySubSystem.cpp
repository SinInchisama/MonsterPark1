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
	if (MainSpawner)
	{
		UE_LOG(LogTemp, Log, TEXT("MainSpawner!"));
	}
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
            }
        }
    }

	MainSpawner->DoDespawning();

}