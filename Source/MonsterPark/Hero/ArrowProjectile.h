// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityTypes.h"
#include "ArrowProjectile.generated.h"

UCLASS()
class MONSTERPARK_API AArrowProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArrowProjectile();

	void ActivateProjectile(FVector InLocation, FRotator InRotation, FMassEntityHandle InTarget, float InDamage);

	void DeactivateProjectile();

	bool IsArrowActive() const { return bIsActive; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;
private:
	FMassEntityHandle TargetEntity;
	float DamageToApply = 0.0f;
	bool bIsActive = false;

	FVector Velocity = FVector::ZeroVector;
	float ArrowSpeed = 3000.0f; 

	class UMassEntitySubsystem* MassEntitySubsystem;
};
