// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterPark/HitInterface.h"
#include "Nexus.generated.h"

UCLASS()
class MONSTERPARK_API ANexusActor : public AActor, public IHitInterface
{
	GENERATED_BODY()

public:
	ANexusActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health = 1000.0f;

	// IHitInterface ±¸Çö
	virtual void TakeMonsterDamage(float DamageAmount, FVector AttackerLocation) override;
	virtual FVector GetTargetLocation(FVector AttackerLocation) override;

private:
	void HandleNexusDestruction();
};
