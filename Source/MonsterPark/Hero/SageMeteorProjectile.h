// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "SageMeteorProjectile.generated.h"

class USceneComponent;
class UNiagaraComponent;
class UPointLightComponent;
class UStaticMeshComponent;
class UMassEntitySubsystem;

UCLASS()
class MONSTERPARK_API ASageMeteorProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASageMeteorProjectile();

	void ActivateProjectile(FVector InLocation, FRotator InRotation, FMassEntityHandle InTarget, float InDamage);
	void DeactivateProjectile();

	bool IsMeteorActive() const { return bIsActive; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeteorRock;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UNiagaraComponent* MeteorTrailFX;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPointLightComponent* MeteorGlow;

	UPROPERTY(EditAnywhere, Category = "Skill")
	float FallDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Skill")
	float MeteorStartHeight = 450.0f;

	UPROPERTY(EditAnywhere, Category = "Skill")
	float ImpactHeight = 0.0f;

private:
	FMassEntityHandle TargetEntity;
	FVector ImpactLocation = FVector::ZeroVector;
	float DamageToApply = 0.0f;
	bool bIsActive = false;
	float FallElapsedTime = 0.0f;

	UPROPERTY()
	UMassEntitySubsystem* MassEntitySubsystem = nullptr;
};
