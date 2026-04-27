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
	float CurrentHealth;	// ���� ü��

	UPROPERTY(EditAnywhere, Category = "Mass|Stats")
	float MaxHealt;			// �ִ� ü��


	UPROPERTY(EditAnywhere, Category = "Mass|Stats")
	int Defenset;			// ����

	UPROPERTY(EditAnywhere, Category = "Mass|Stats")
	int BaseDefense = 0;

	UPROPERTY(EditAnywhere, Category = "Mass|Stats")
	float SpeedMultiplier = 400.0f;

	UPROPERTY(EditAnywhere, Category = "Mass|Stats")
	float PendingAoEDamage = 0.0f;
};
