// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PlayState.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API APlayState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY( BlueprintReadOnly, Category = "Match")
	int32 CurrentRound;

	UPROPERTY( BlueprintReadOnly, Category = "Match")
	int32 RemainingTime;
};
