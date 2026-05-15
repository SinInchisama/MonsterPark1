// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "ArchAngel.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API AArchAngel : public ACharacterBase
{
	GENERATED_BODY()

public:
	AArchAngel();
	virtual void Tick(float DeltaTime) override;
	virtual void UseSkill() override;
	bool ExecuteSkill();
	bool IsSkillRequested() const;
	void ClearSkillRequest();
	bool HasMinionClass() const;

protected:
	virtual void FindEnemiesInArea() override;
	virtual UAnimMontage* GetDetectedMontage() const override;
	virtual void PlayDetectedMontageIfNeeded() override;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* ArchAngelFullMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* SkillMontage;

	UPROPERTY(EditAnywhere, Category = "Skill")
	TSubclassOf<ACharacterBase> MinionClass;

	UPROPERTY(EditAnywhere, Category = "Skill")
	float MinionLifetime = 40.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float BuffRadius = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackSpeedBuffMultiplier = 1.5f;

	bool bHasPlayedPassive = false;
	bool bSkillRequested = false;
};
