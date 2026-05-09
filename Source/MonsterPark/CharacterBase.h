// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "MassEntityQuery.h"
#include "DetectionUnitTypes.h"
#include "MyAnimInstance.h"
#include "HitInterface.h"
#include "CharacterBase.generated.h"

class UBoxComponent;
class UAnimMontage;
class UAnimInstance;
class UMyAnimInstance;
class UNiagaraSystem;
class UGameplayAbility;

UCLASS(Blueprintable, BlueprintType, ShowCategories=("Animation", "Collision"))
class MONSTERPARK_API ACharacterBase : public ACharacter, public IAbilitySystemInterface, public IHitInterface
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
	TSubclassOf<UGameplayAbility> SkillAbilityClass;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UNiagaraSystem* HitEffectTemplate;

	int64 LastGridKey = -1;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
	EGameplayEffectReplicationMode AscReplicationMode = EGameplayEffectReplicationMode::Mixed;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	void Attack_Melee();
	void Attack_End();
	void UseSkill();
	
	virtual void FindEnemiesInArea();

	void MoveForward(float val);
	void MoveRight(float val);

	void UpdateAnimBPSpeed(int val);

	void SetIsOutside(bool bOutside);

	virtual void TakeMonsterDamage(float Damage, FVector AttackerLocation) override;

	virtual FVector GetTargetLocation(FVector AttackerLocation) override {return GetActorLocation();};

	void SetSelectedHero(bool bIsSelected);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UMyAnimInstance> AnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* AnimMontage = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	UBoxComponent* SelectionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Selection")
	class UDecalComponent* SelectionDecal;

    TArray<FMassEntityHandle> DetectedEnemies;
    FMassEntityQuery EnemyQuery;
	FMassEntityQuery EnemyOutsideQuery;

	FMassEntityQuery* TargetQueryPtr = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class UTexture2D* UnitPortrait;
protected:
	void OnResumeAction();
	void GrantDefaultAbilities();

	FTimerHandle AttackDelayTimerHandle;
	FTimerHandle TH_Attack_End;

	bool Attacking = true;
	bool bEnemyDetected = false;

	virtual void PlayDetectedMontageIfNeeded();
	void SetMoveAnimClassIfNeeded();
	void Attack();
	void ResetAttack();
	virtual UAnimMontage* GetDetectedMontage() const;
	TSubclassOf<UAnimInstance> GetMoveAnimClass() const;

	float CurrentForwardInput = 0;
	float CurrentRightInput = 0;

	bool bIsOutsideWall;
	UPROPERTY()
	class UPlaySubSystem* PlaySubsystem;
};
