// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PlaySubSystem.h"
#include "CharacterBase.h"
#include "HeroChanceRow.h"
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

USTRUCT(BlueprintType)
struct FSpecialRecipe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FString MainHeroName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 MainHeroCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FString SubHeroName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 SubHeroCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TSubclassOf<class ACharacterBase> ResultHeroClass;
};

UCLASS()
class MONSTERPARK_API ABasicGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ABasicGameMode();

	UFUNCTION(BlueprintCallable)
	void SpawnHeroFromShop(TSubclassOf<ACharacterBase> HeroClass, ACharacter* PlayerChar);

	UFUNCTION(BlueprintCallable)
	void OnLevelUp(FHeroChanceRow& CurrentChane, int Level);

	UFUNCTION()
	void GameOver(bool bVictory);

	UFUNCTION(BlueprintCallable)
	TSubclassOf<ACharacterBase>GetRandomHeroByChance(const FHeroChanceRow& ChanceData);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterSetup")
	TArray<TSubclassOf<ACharacterBase>> MonsterOneCoinClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterSetup")
	TArray<TSubclassOf<ACharacterBase>> MonsterTwoCoinClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterSetup")
	TArray<TSubclassOf<ACharacterBase>> MonsterThreeCoinClasses;

	UPROPERTY(BlueprintAssignable, Category = "GameRules")
	FOnTimerUpdated OnTimerUpdated;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TArray<FSpecialRecipe> SpecialRecipes;

	void TrySpecialMixture(class AMyBasicCharacter* Player, class ACharacterBase* SelectedHero);

	void ToggleMenuUI();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void UpdateMatchState(EMatchState NewState);

	UFUNCTION()
	void UpdateTimerEverySecond();

	UFUNCTION()
	void Mixture(int32 cost);

	UFUNCTION()
	void TryStartTimer();

	UPlaySubSystem* MonsterSubsystem;

	UPROPERTY()
	float RoundTimer = 10;
	FTimerHandle TimerHandle; 
	FTimerHandle SpawnerCheckHandle;

	UPROPERTY()
	float StayTime = 10;
	UPROPERTY()
	float PlayTime = 40;


	UPROPERTY()
	EMatchState CurrentState;

	UPROPERTY(EditAnywhere, Category = "Data")
	UDataTable* HeroChanceTable;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> MenuWidgetClass;

	UPROPERTY()
	UUserWidget* CurrentMenuWidget;
};
