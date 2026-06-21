// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterPark/Monster/EntityHealthInterface.h"
#include "MonsterVisualActor.generated.h"

class UWidgetComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class MONSTERPARK_API AMonsterVisualActor : public AActor, public IEntityHealthInterface
{
	GENERATED_BODY()

public:
	AMonsterVisualActor();

	virtual void UpdateHealthUI_Implementation(float CurrentHealth) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* DummyRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* HealthBarComponent;
};