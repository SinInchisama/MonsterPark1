// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Shielder.generated.h"

/**
 * 
 */

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class MONSTERPARK_API AShielder : public ACharacterBase
{
	GENERATED_BODY()
	
public:
    AShielder();

	virtual void Tick(float DeltaTime) override;

    virtual void BeginPlay() override;

protected:
    // 1. 부모의 적 감지 로직에 슬로우 효과를 추가하기 위해 오버라이드
    virtual void FindEnemiesInArea() override;

    // 2. 부모 클래스가 재생할 몽타주를 물어볼 때 이 변수를 반환
    virtual UAnimMontage* GetDetectedMontage() const override;

    // 에디터에서 Shielder 전용 몽타주 할당
    UPROPERTY(EditAnywhere, Category = "Animation")
    UAnimMontage* ShielderFullMontage;

    // 슬로우 수치 설정
    UPROPERTY(EditAnywhere, Category = "Combat")
    float SlowMultiplier = 200.0f;

	virtual void PlayDetectedMontageIfNeeded() override;

    bool bHasPlayedPassive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UNiagaraSystem* AuraTemplate;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
    UNiagaraComponent* AuraComponent;

    TArray<FMassEntityHandle> SlowedMonsters;
};
