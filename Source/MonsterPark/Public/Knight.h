// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HeroActor.h"
#include "Knight.generated.h"

UCLASS()
class MONSTERPARK_API AKnight : public AHeroActor
{
	GENERATED_BODY()

public:
	AKnight();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
