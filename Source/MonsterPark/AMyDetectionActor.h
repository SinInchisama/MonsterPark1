// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassEntityQuery.h"
#include "MyAnimInstance.h"
class UAnimMontage;
class UAnimInstance;
#include "AMyDetectionActor.generated.h"

UENUM(BlueprintType)
enum class EDetectionUnitType : uint8
{
	BlackCat,
	Knight,
	Thief,
	Archer,
};

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
	bool bEnemyDetected = false;

	void PlayDetectedMontageIfNeeded();
	void SetMoveAnimClassIfNeeded();
	UAnimMontage* GetDetectedMontage() const;
	TSubclassOf<UAnimInstance> GetMoveAnimClass() const;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void FindEnemiesInArea();

	void MoveForward(int val);
	void MoveRight(int val);

	void UpdateAnimBPSpeed(int val);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EDetectionUnitType UnitType = EDetectionUnitType::BlackCat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UMyAnimInstance> BlackCatMoveAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UMyAnimInstance> KnightMoveAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UMyAnimInstance> ThiefMoveAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UMyAnimInstance> ArcherMoveAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* BlackCatAnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* KnightAnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ThiefAnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ArcherAnimMontage = nullptr;



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	class UBoxComponent* SelectionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroStats")
	int32 HeroPrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroStats")
	FText HeroDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroStats")
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroStats")
	int32 Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeroStats")
	float AttackSpeed;

	float CurrentForwardInput = 0;
	float CurrentRightInput = 0;
};
