// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterPark/HitInterface.h"
#include "WallActor.generated.h"

UCLASS()
class MONSTERPARK_API AWallActor : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWallActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class UBoxComponent* AttackZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health = 500.0f;

	virtual void TakeMonsterDamage(float DamageAmount, FVector AttackerLocation) override;

	virtual FVector GetTargetLocation(FVector AttackerLocation) override;
};
