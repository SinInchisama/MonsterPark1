// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Sage.generated.h"

class ASageMeteorProjectile;
class UNiagaraSystem;

/**
 * 
 */
UCLASS()
class MONSTERPARK_API ASage : public ACharacterBase
{
	GENERATED_BODY()

public:
	ASage();
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
	UAnimMontage* SageFullMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* SkillMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float SplashRadius = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Combat|VFX")
	UNiagaraSystem* BasicAttackFirePillarTemplate = nullptr;

	UPROPERTY(EditAnywhere, Category = "Combat|VFX")
	float BasicAttackFirePillarDuration = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Combat|VFX")
	float BasicAttackFirePillarZOffset = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float StunRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float StunDuration = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float StunSpeedMultiplier = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Skill")
	float SkillDamage = 10000.0f;

	UPROPERTY(EditAnywhere, Category = "Skill")
	float SkillRadius = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Skill")
	TSubclassOf<ASageMeteorProjectile> MeteorClass;

	UPROPERTY(EditAnywhere, Category = "Skill")
	int32 MeteorPoolSize = 5;

	bool bHasPlayedPassive = false;
	bool bSkillRequested = false;

private:
	ASageMeteorProjectile* GetAvailableMeteor();
	void SpawnBasicAttackFirePillar(const FVector& TargetLocation);
	void ApplyStun();
	void ResetStun();

	FTimerHandle StunResetHandle;
	TArray<FMassEntityHandle> StunnedEntities;

	UPROPERTY()
	TArray<ASageMeteorProjectile*> MeteorPool;
};
