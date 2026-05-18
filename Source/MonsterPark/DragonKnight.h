// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "DragonKnight.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API ADragonKnight : public ACharacterBase
{
	GENERATED_BODY()

public:
	ADragonKnight();
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
	UAnimMontage* DragonKnightFullMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* SkillMontage;

	UPROPERTY(EditAnywhere, Category = "Skill")
	float SkillDamage = 5000.0f;

	UPROPERTY(EditAnywhere, Category = "Skill")
	float BreathDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Skill")
	float BreathTickInterval = 1.0f;

	bool bHasPlayedPassive = false;
	bool bSkillRequested = false;

private:
	void StartBreathDamage();
	void ApplyBreathDamageTick();

	FTimerHandle BreathTimerHandle;
	int32 RemainingBreathTicks = 0;
	int32 TotalBreathTicks = 0;
	float BreathDamagePerTick = 0.0f;
};
