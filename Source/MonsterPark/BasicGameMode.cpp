// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicGameMode.h"
#include  "Game_HUD.h"
#include "MyBasicCharacter.h"
#include "PlaySubSystem.h"

ABasicGameMode::ABasicGameMode()
{
	HUDClass = AGame_HUD::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	CurrentRound = 0;
	//PrimaryActorTick.bStartWithTickEnabled = true;
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

void ABasicGameMode::BeginPlay()
{
	Super::BeginPlay();

	MonsterSubsystem = GetWorld()->GetSubsystem<UPlaySubSystem>();

	if (MonsterSubsystem)
	{
		if (MonsterSubsystem->MainSpawner)
		{
			UE_LOG(LogTemp, Log, TEXT("start spawn!"));
			RoundTimer = 5.f;
		}
	}
	CurrentState = EMatchState::Waiting;
}

void ABasicGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RoundTimer -= DeltaTime;

	if (RoundTimer <= 0.0f)
	{
		if (CurrentState == EMatchState::Waiting) UpdateMatchState(EMatchState::Playing);
		else UpdateMatchState(EMatchState::Waiting);
	}
}

void ABasicGameMode::UpdateMatchState(EMatchState NewState)
{
	CurrentState = NewState;

	if (CurrentState == EMatchState::Waiting)
	{
		// 1. 다음 라운드 데이터 준비 (서브시스템 활용)
		RoundTimer = 5.0f;
		MonsterSubsystem->MainSpawner->DoDespawning();
		++CurrentRound;
	}
	else if (CurrentState == EMatchState::Playing)
	{
		// 2. 대기 중인 몬스터들 깨우기
		if (MonsterSubsystem->MainSpawner)
		{
			MonsterSubsystem->StartRound(CurrentRound,1);
		}
		RoundTimer = 5.0f;
	}
}