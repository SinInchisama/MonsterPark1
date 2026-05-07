// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Lancer.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API ALancer : public ACharacterBase
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void FindEnemiesInArea() override;
	virtual UAnimMontage* GetDetectedMontage() const override;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* LancerFullMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float BleedPercent = 0.02f;
};
