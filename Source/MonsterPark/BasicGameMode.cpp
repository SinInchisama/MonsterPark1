// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicGameMode.h"
#include  "Game_HUD.h"
#include "MyBasicCharacter.h"

void ABasicGameMode::AMyGameModeBase()
{
	HUDClass = AGame_HUD::StaticClass();
}

void ABasicGameMode::SpawnHeroFromShop(TSubclassOf<AActor> HeroClass, ACharacter* PlayerChar)
{
	if (!HeroClass) return;

	FVector SpawnLocation(0.0f, 0.0f, 0.0f);
	FRotator SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters SpawnParams;

	AActor* NewHero = GetWorld()->SpawnActor<AActor>(HeroClass, SpawnLocation, SpawnRotation, SpawnParams);

	AMyBasicCharacter* MyChar = Cast<AMyBasicCharacter>(PlayerChar);
	if (MyChar)
	{
		MyChar->SetSummonedActor(NewHero);
	}
}
