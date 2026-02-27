// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicGameMode.h"
#include  "Game_HUD.h"
#include "MyBasicCharacter.h"
#include "CharacterBase.h"

void ABasicGameMode::AMyGameModeBase()
{
	HUDClass = AGame_HUD::StaticClass();
}

void ABasicGameMode::SpawnHeroFromShop(TSubclassOf<ACharacterBase> HeroClass, ACharacter* PlayerChar)
{
	if (!HeroClass) return;

	FVector SpawnLocation(0.0f, 0.0f, 110.0f);
	FRotator SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters SpawnParams;

	ACharacterBase* NewHero = GetWorld()->SpawnActor<ACharacterBase>(HeroClass, SpawnLocation, SpawnRotation, SpawnParams);

	AMyBasicCharacter* MyChar = Cast<AMyBasicCharacter>(PlayerChar);
	if (MyChar)
	{
		MyChar->SetSummonedActor(NewHero);
	}
}
