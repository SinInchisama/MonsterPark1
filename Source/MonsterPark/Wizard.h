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
	virtual UAnimMontage* GetDetectedMontage() const override;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* WizardFullMontage;

	// 광역스킬
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AoESkill = 0.f;

	virtual void PlayDetectedMontageIfNeeded() override;

	bool bHasPlayedPassive = false;
};
