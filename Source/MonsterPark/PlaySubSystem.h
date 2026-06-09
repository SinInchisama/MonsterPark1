// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MyMassSpawner.h"
#include "MassEntityTypes.h"
#include "PlaySubSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundChanged, int32, NewRound);
/**
 * 
 */

USTRUCT()
struct FMonsterGridInfo
{
	GENERATED_BODY()

	FMassEntityHandle MonsterHandle;

	FVector Location;
};

USTRUCT()
struct FGridData
{
	GENERATED_BODY()
	TArray<AActor*> HeroesInCell;
	TArray<FMonsterGridInfo> MonsterInfos;
};

UCLASS()
class MONSTERPARK_API UPlaySubSystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void StartRound(int Round,int Scale);
	void EndRound();
protected:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

public:
	UPROPERTY()
	TArray<AMyMassSpawner*> MainSpawners;

	UPROPERTY()
	int32 CurrentRound;

	UPROPERTY()
	TArray<AActor*> ActiveWalls;
	UPROPERTY()
	AActor* Nexus;


	UPROPERTY(BlueprintAssignable, Category = "Game")
	FOnRoundChanged OnRoundChanged;

	const float CellSize = 500.f;

	TMap<int64, FGridData> SpatialGrid;

	int64 GetGridKey(FVector Location) const
	{
		int32 X = FMath::FloorToInt(Location.X / CellSize);
		int32 Y = FMath::FloorToInt(Location.Y / CellSize);
		return ((int64)X << 32) | (uint32)Y; // X, Y를 합쳐 하나의 키로 만듦
	}

	TMap<FIntVector, int32> ObstacleMap;

	void UpdateHeroLocation(AActor* Hero, int64& InOutLastKey, FVector NewLocation);
	void UpdateMonsterLocation(FMassEntityHandle Entity, int64& InOutLastKey, FVector NewLocation);
	void RemoveHeroFromGrid(AActor* Hero, int64& InOutLastKey);
	AActor* FindNearestHeroInGrid(FVector SearchLocation, float SearchRadius);

	AActor*  FindFinalRoundTarget(FVector MonsterLocation);
	void OnWallDestroyed(AActor* DestroyedWall);

	FIntVector PosToGrid(const FVector& Pos) const;
	bool IsGridBlocked(const FIntVector& GridKey) const;
	void RegisterObstacle(const TArray<FIntVector>& GridKeys);

	FVector GridToPos(const FIntVector& GridKey) const;
	TArray<FIntVector> FindPath(const FIntVector& StartGrid, const FIntVector& TargetGrid);
	bool HasLineOfSight(const FIntVector& Start, const FIntVector& End) const;
	TArray<FIntVector> SmoothPath(const TArray<FIntVector>& InPath);

	UPROPERTY()
	TMap<FName, int32> GlobalHeroUpgradeLevels;

	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	int32 GetHeroUpgradeLevel(FName HeroName) const;

	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void IncreaseHeroUpgradeLevel(FName HeroName);
};