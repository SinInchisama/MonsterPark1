// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaySubSystem.h"
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