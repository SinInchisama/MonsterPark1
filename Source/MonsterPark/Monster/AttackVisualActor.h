// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EntityHealthInterface.h"
#include "AttackVisualActor.generated.h"

class UWidgetComponent;
class USceneComponent;
class USkeletalMeshComponent;

UCLASS()
class MONSTERPARK_API AAttackVisualActor : public AActor, public IEntityHealthInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAttackVisualActor();

	virtual void UpdateHealthUI_Implementation(float HealthPercent) override;

void PlayAttackAnimation();
void PlayDeathAnimation();
protected:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* HealthBarComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* DummyRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<class UAnimMontage*> RandomAttackMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* DeathMontage;

};
