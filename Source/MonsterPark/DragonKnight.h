// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "DragonKnight.generated.h"

class AActor;
class UParticleSystem;

/**
 * 
 */
UCLASS()
class MONSTERPARK_API ADragonKnight : public ACharacterBase
{
	GENERATED_BODY()

public:
	ADragonKnight();
	virtual void BeginPlay() override;
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
	float BurnPercent = 0.01f;

	UPROPERTY(EditAnywhere, Category = "Skill")
	float BreathDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Skill")
	float BreathTickInterval = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Skill|VFX")
	TSubclassOf<AActor> DragonBreathVFXClass;

	UPROPERTY(EditAnywhere, Category = "Skill|VFX")
	FName DragonBreathSocketName = FName("DragonMouth");

	UPROPERTY(EditAnywhere, Category = "Skill|VFX")
	FVector DragonBreathFallbackOffset = FVector(250.0f, 0.0f, 180.0f);

	UPROPERTY(EditAnywhere, Category = "Skill|VFX")
	float DragonBreathVFXLifeTime = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill|Effects")
	UParticleSystem* BasicAttackFireStormTemplate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill|Effects")
	FVector BasicAttackFireStormOffset = FVector(250.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill|Effects")
	FVector BasicAttackFireStormScale = FVector(1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill|Effects")
	float BasicAttackFireStormDuration = 3.0f;

	bool bHasPlayedPassive = false;
	bool bSkillRequested = false;

private:
	void SpawnDragonBreathVFX();
	void StartBreathDamage();
	void ApplyBreathDamageTick();

	FTimerHandle BreathStartTimerHandle;
	FTimerHandle BreathTimerHandle;
	int32 RemainingBreathTicks = 0;
	int32 TotalBreathTicks = 0;
};
