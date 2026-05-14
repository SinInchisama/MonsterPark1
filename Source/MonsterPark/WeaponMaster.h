// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "WeaponMaster.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API AWeaponMaster : public ACharacterBase
{
	GENERATED_BODY()

public:
	AWeaponMaster();
	virtual void Tick(float DeltaTime) override;
	virtual void UseSkill() override;
	bool ExecuteSkill();
	bool IsSkillRequested() const;
	void ClearSkillRequest();

protected:
	virtual void FindEnemiesInArea() override;
	virtual UAnimMontage* GetDetectedMontage() const override;
	virtual void PlayDetectedMontageIfNeeded() override;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* WeaponMasterFullMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* SkillMontage;

	UPROPERTY()
	UAnimMontage* WeaponMasterSkillMontage_DEPRECATED = nullptr;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float SplashRadius = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float CriticalChance = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float CriticalDamageMultiplier = 1.6f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float SkillDamage = 50000.0f;

	bool bHasPlayedPassive = false;
	bool bSkillRequested = false;
};
