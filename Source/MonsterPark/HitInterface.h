// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHitInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MONSTERPARK_API IHitInterface
{
	GENERATED_BODY()

public:
	virtual void TakeMonsterDamage(float Damage, FVector AttackerLocation) = 0;

	virtual FVector GetTargetLocation(FVector AttackerLocation) { return FVector::ZeroVector; }
};
