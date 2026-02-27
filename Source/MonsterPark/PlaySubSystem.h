// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassSpawner.h"
#include "PlaySubSystem.generated.h"


/**
 * 
 */
UCLASS()
class MONSTERPARK_API UPlaySubSystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void StartRound(float Scale);
protected:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

public:
	UPROPERTY()
	AMassSpawner* MainSpawner;
};
