// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "MassEntityQuery.h"
#include "DetectionUnitTypes.h"
#include "MyAnimInstance.h"
#include "CharacterBase.generated.h"

class UBoxComponent;
class UAnimMontage;
class UAnimInstance;
class UMyAnimInstance;

UCLASS(Blueprintable, BlueprintType, ShowCategories=("Animation", "Collision"))
class MONSTERPARK_API ACharacterBase : public APawn, public IAbilitySystemInterface
{
    GENERATED_BODY()
    
public:	
	// Sets default values for this actor's properties
	ACharacterBase();

	// Ability System Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	class UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	class UMonsterAttributeSet* AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	FText UnitName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
	float DefaultAttackPower = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
	float DefaultAttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
	float DefaultRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit|Stats")
	float DefaultCost = 1.0f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
	EGameplayEffectReplicationMode AscReplicationMode = EGameplayEffectReplicationMode::Mixed;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//virtual void PossessedBy(AController* NewController) override;

	//virtual void OnRep_PlayerState() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	void Attack_Melee();
	void Attack_End();
	
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
	TSubclassOf<UMyAnimInstance> WizardMoveAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UMyAnimInstance> BerserkerMoveAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UMyAnimInstance> FighterMoveAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UMyAnimInstance> ShielderMoveAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* BlackCatAnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* KnightAnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ThiefAnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ArcherAnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* WizardAnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* BerserkerAnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* FighterAnimMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ShielderAnimMontage = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	UBoxComponent* SelectionBox;

    TArray<FMassEntityHandle> DetectedEnemies;
    FMassEntityQuery EnemyQuery;

protected:
	void OnResumeAction();

	FTimerHandle AttackDelayTimerHandle;
	FTimerHandle TH_Attack_End;

	bool Attacking = true;
	bool bEnemyDetected = false;

	void PlayDetectedMontageIfNeeded();
	void SetMoveAnimClassIfNeeded();
	void Attack();
	void ResetAttack();
	UAnimMontage* GetDetectedMontage() const;
	TSubclassOf<UAnimInstance> GetMoveAnimClass() const;

	float CurrentForwardInput = 0;
	float CurrentRightInput = 0;
};
