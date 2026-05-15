// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "ArchAngelMinion.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API AArchAngelMinion : public ACharacterBase
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Follow")
	float FollowDistance = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Follow")
	float FollowSpeed = 300.0f;
};
