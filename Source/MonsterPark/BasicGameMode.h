// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BasicGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API ABasicGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	void AMyGameModeBase();

	UFUNCTION(BlueprintCallable)
	void SpawnHeroFromShop(TSubclassOf<AActor> HeroClass, ACharacter* PlayerChar);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MonsterSetup")
	TArray<TSubclassOf<AActor>> MonsterClasses;

protected:
	virtual void BeginPlay() override;

};
