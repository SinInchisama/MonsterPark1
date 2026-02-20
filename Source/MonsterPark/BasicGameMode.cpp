// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicGameMode.h"
#include  "Game_HUD.h"

void ABasicGameMode::AMyGameModeBase()
{
	HUDClass = AGame_HUD::StaticClass();
}
