// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Wyvern.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API AWyvern : public ACharacterBase
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void FindEnemiesInArea() override;
	virtual UAnimMontage* GetDetectedMontage() const override;
	virtual void PlayDetectedMontageIfNeeded() override;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* WyvernFullMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float SplashRadius = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float BurnPercent = 0.01f;

	bool bHasPlayedPassive = false;
};
