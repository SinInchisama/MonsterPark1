// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Witch.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API AWitch : public ACharacterBase
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void FindEnemiesInArea() override;
	virtual UAnimMontage* GetDetectedMontage() const override;
	virtual void PlayDetectedMontageIfNeeded() override;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* WitchFullMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float SlowMultiplier = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DefenseReductionMultiplier = 0.5f;

	bool bHasPlayedPassive = false;
};
