// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "MassEntityTypes.h"
#include "Archer.generated.h"

/**
 * 
 */
class AArrowProjectile;

UCLASS()
class MONSTERPARK_API AArcher : public ACharacterBase
{
	GENERATED_BODY()


protected:
	virtual void BeginPlay() override;

	// ?? 부모의 FindEnemiesInArea를 가로채서 대미지 처리 방식을 화살 스폰으로 대체합니다.
	virtual void FindEnemiesInArea() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<AArrowProjectile> ArrowClass;

	UPROPERTY(EditAnywhere, Category = "Combat")
	int32 ArrowPoolSize = 10;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ArrowFireDelay = 0.3f;

private:
	void ExecuteDelayedFire();

	FTimerHandle FireTimerHandle;

	UPROPERTY()
	TArray<AArrowProjectile*> LoadedArrows;

	// 전역 화살 풀링 보관함
	UPROPERTY()
	TArray<AArrowProjectile*> ArrowPool;
};
