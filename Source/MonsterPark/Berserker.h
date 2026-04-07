// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Berserker.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API ABerserker : public ACharacterBase
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual UAnimMontage* GetDetectedMontage() const override;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* BerserkerFullMontage;
};