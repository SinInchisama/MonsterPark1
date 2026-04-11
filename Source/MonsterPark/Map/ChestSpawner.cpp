// Fill out your copyright notice in the Description page of Project Settings.

#include "ChestSpawner.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TreasureChest.h"

// Sets default values
AChestSpawner::AChestSpawner()
{

}

// Called when the game starts or when spawned
void AChestSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnChests();
}

void AChestSpawner::SpawnChests()
{
	if (!ChestClass) return;

    for (int32 i = 0; i < ChestCount; ++i)
    {
        float RandomX = FMath::FRandRange(-12600.0f, 12600.0f);
        float RandomY = FMath::FRandRange(-12600.0f, 12600.0f);

        FVector SpawnerLocation = GetActorLocation();
        FVector TraceStart = FVector(SpawnerLocation.X + RandomX, SpawnerLocation.Y + RandomY, 10000.0f);
        FVector TraceEnd = FVector(TraceStart.X, TraceStart.Y, -10000.0f);

        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            GetWorld()->SpawnActor<ATreasureChest>(ChestClass, HitResult.Location, FRotator::ZeroRotator);
        }
    }
}



