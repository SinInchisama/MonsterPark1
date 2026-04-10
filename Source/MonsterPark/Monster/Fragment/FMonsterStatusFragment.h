// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "FMonsterStatusFragment.generated.h"

/**
 * 
 */
USTRUCT()
struct MONSTERPARK_API FMonsterStatusFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Mass|Stats")
	float CurrentHealth;	// 현재 체력

	UPROPERTY(EditAnywhere, Category = "Mass|Stats")
	float MaxHealt;			// 최대 체력


	UPROPERTY(EditAnywhere, Category = "Mass|Stats")
	int Defenset;			// 방어력

	UPROPERTY(EditAnywhere, Category = "Mass|Stats")
	float SpeedMultiplier = 400.0f;
};
