// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Thief.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API AThief : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;
};
