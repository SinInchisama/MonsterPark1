// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Wyvern.generated.h"

class UParticleSystem;

/**
 * 
 */
UCLASS()
class MONSTERPARK_API AWyvern : public ACharacterBase
{
	GENERATED_BODY()

public:
	AWyvern();
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Effects")
	UParticleSystem* BreathTemplate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Effects")
	FName BreathSocketName = FName("Mouth");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Effects")
	FVector BreathFallbackOffset = FVector(130.0f, 0.0f, 110.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Effects")
	FVector BreathScale = FVector(1.5f, 0.6f, 0.6f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Effects")
	float BreathLifeTime = 1.5f;

	bool bHasPlayedPassive = false;

private:
	void SpawnBreathVFX();
};
