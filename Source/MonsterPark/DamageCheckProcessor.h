// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "DamageCheckProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API UDamageCheckProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UDamageCheckProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	UPROPERTY()
	FMassEntityQuery DamageCheckQuery;
};
