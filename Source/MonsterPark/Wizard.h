// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Wizard.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API AWizard : public ACharacterBase
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;

protected:
	virtual void FindEnemiesInArea() override;
    virtual UAnimMontage* GetDetectedMontage() const override;
    virtual void PlayDetectedMontageIfNeeded() override;

    UPROPERTY(EditAnywhere, Category = "Animation")
    UAnimMontage* WizardFullMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float SplashRadius = 250.0f;

    bool bHasPlayedPassive = false;
};