// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PlaySubSystem.h"
#include "CharacterBase.h"
#include "BasicGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerUpdated, float, RemainingTime);
/**
 * 
 */
UENUM(BlueprintType)
enum class EMatchState : uint8
{
	Waiting,    // ��� ���� (���� ���� �� ���) 
	Playing,    // �÷��� ���� (���� ����)
	GameOver    // ���� ����
};

UCLASS()
class MONSTERPARK_API ABasicGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABasicGameMode();

	UFUNCTION(BlueprintCallable)
	void SpawnHeroFromShop(TSubclassOf<ACharacterBase> HeroClass, ACharacter* PlayerChar);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterSetup")
	TArray<TSubclassOf<ACharacterBase>> MonsterOneCoinClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterSetup")
	TArray<TSubclassOf<ACharacterBase>> MonsterTwoCoinClasses;

	UPROPERTY(BlueprintAssignable, Category = "GameRules")
	FOnTimerUpdated OnTimerUpdated;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void UpdateMatchState(EMatchState NewState);

	UFUNCTION()
	void UpdateTimerEverySecond();

	UPlaySubSystem* MonsterSubsystem;

	UPROPERTY()
	float RoundTimer = 5;
	FTimerHandle TimerHandle;

	UPROPERTY()
	int32 CurrentRound;

	UPROPERTY()
	float StayTime = 5;
	UPROPERTY()
	float PlayTime = 20;


	UPROPERTY()
	EMatchState CurrentState;
};