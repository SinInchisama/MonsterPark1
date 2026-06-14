// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.h"
#include "HeroChanceRow.h"
#include "MyBasicCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLifeChanged, int32, NewLife);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMixtureHero, int32, HeroCost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitSelectedSignature, ACharacterBase*, SelectedUnit, bool, Select);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNexusToggledSignature, bool, bIsOpen);

UCLASS()
class MONSTERPARK_API AMyBasicCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyBasicCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GameraMoveForward(float value);
	void GameraMoveRight(float value);

	void HeroMoveForward(float value);
	void HeroMoveRight(float value);
	void OnSkillPressed();

	void Set_PlayerMoney(int32 value);
	int32 Get_PlayerMoney();

	int32 Get_PlayerLife();
	void Miu_PlayerLife(int32 value);

	int32 Get_PlayerExp();
	void Set_PlayerExp(int32 value);
	bool CheckLevelUp();

	bool PlayerLevelUp();

	void SetSummonedActor(AActor* InActor);

	void OnLeftClickPressed();
	void OnLeftClickReleased();
	void OnMouseRightClick();

	UFUNCTION(BlueprintCallable)
	void SetPrimarySelectedHero(ACharacterBase* NewPrimaryHero);

	void HeroMixture();
	FVector2D StartMousePosition;
	FVector2D EndMousePosition;

	void OpenMenu();

	UPROPERTY(EditAnywhere, Category = "PostProcess")
	class APostProcessVolume* MyPPVolume;

	bool bIsHeroInvincible = false;

	bool bIsInvincible = false;

	void TogglePPVolume();
	void OnToggleTimerPressed(); 
	void InputCheatReduceTimer();
	void InputCheatPlusTimer();
	void InputCheatToggleInvincible();
	void InputCheatTeleportUnit();
	void InputCheatHeroInvincible();
	void InputCheatAddMoney();
	void InputCheatMaxRound();
protected:
	FTimerHandle TH_Attack_End;

	UPROPERTY()
	class AGame_HUD* MyHUD;
public:
	// ���� ī�޶� �κ�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(BlueprintAssignable)
	FOnLifeChanged OnLifeChanged;

	UPROPERTY(BlueprintAssignable)
	FMixtureHero Mixtured;

	UPROPERTY(BlueprintAssignable)
	FOnUnitSelectedSignature OnUnitSelected;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNexusToggledSignature OnNexusToggled;

	FHeroChanceRow CurrentLevelChance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<ACharacterBase*> MySummonedHero;

public:
	// ���� �÷��̾� ��, ����ġ, ������, ������ �ִ� �������� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Player)
	int32 PlayerMoney = 20;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Player)
	int32 PlayerLife = 20;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Player)
	int32 PlayerExp = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Player)
	int32 PlayerMaxExp = 14;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Player)
	int32 PlayerLevel = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Leveling")
	TArray<int32> LevelRequiredExp;

	UPROPERTY()
	TArray<class ACharacterBase*> SelectedHeroes;

	ACharacterBase* SelectHero;
};
