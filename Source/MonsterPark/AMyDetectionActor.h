// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityQuery.h"
class UAnimMontage;
#include "AMyDetectionActor.generated.h"

UCLASS()
class MONSTERPARK_API AAMyDetectionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	TArray<FMassEntityHandle> DetectedEnemies;
	FMassEntityQuery EnemyQuery;
	// Sets default values for this actor's properties
	AAMyDetectionActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OnResumeAction();

	FTimerHandle DetectionTimerHandle;

	bool Attacking = true;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void FindEnemiesInArea();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* BlackCatAnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* KnightAnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroStats")
	int32 HeroPrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroStats")
	FText HeroDisplayName;
};
