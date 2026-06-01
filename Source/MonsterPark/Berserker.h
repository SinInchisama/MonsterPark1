// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Berserker.generated.h"

/**
 * 
 */

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class MONSTERPARK_API ABerserker : public ACharacterBase
{
	GENERATED_BODY()

public:
	ABerserker();

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

protected:
	virtual void FindEnemiesInArea() override;
	virtual UAnimMontage* GetDetectedMontage() const override;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* BerserkerFullMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float DefenseReductionMultiplier = 0.7f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraComponent* AuraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UNiagaraSystem* AuraTemplate;

	TArray<FMassEntityHandle> DebuffedMonsters;
};