// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PlaySubSystem.h"
#include "CharacterBase.h"
#include "BasicGameMode.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EMatchState : uint8
{
	Waiting,    // 대기 라운드 (몬스터 생성 및 대기)
	Playing,    // 플레이 라운드 (전투 시작)
	GameOver    // 게임 종료
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
	TArray<TSubclassOf<ACharacterBase>> MonsterClasses;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void UpdateMatchState(EMatchState NewState);

	UPlaySubSystem* MonsterSubsystem;

	UPROPERTY()
	float RoundTimer;

	UPROPERTY()
	int32 CurrentRound;

	UPROPERTY()
	EMatchState CurrentState;
};
