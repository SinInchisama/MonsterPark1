// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Game_HUD.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API AGame_HUD : public AHUD
{
	GENERATED_BODY()
	
public:
	AGame_HUD();

	void BeginPlay() override;

private:
	TSubclassOf<class UUserWidget> uiBPClass;
	class UUserWidget* uiWidget;
};
