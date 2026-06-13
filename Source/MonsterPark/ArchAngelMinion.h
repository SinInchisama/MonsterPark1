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
	AArchAngelMinion();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Follow")
	float FollowAcceptanceRadius = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Follow")
	float FollowSpeed = 300.0f;

	FVector InitialRelativeOffset = FVector::ZeroVector;
	bool bHasInitialRelativeOffset = false;
};
