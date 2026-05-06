// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LoadingGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API ALoadingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	float GetLoadingProgress();

protected:
	virtual void BeginPlay() override;

	void StartAsyncLevelLoad();

	UFUNCTION()
	void OnLevelLoaded();

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	UPROPERTY()
	UUserWidget* CurrentWidget;
};
