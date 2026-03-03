// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicGameMode.h"
#include "Game_HUD.h"
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
			RoundTimer = StayTime;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &ABasicGameMode::UpdateTimerEverySecond, 1.0f, true);

			OnTimerUpdated.Broadcast(RoundTimer);
		}
	}
	CurrentState = EMatchState::Waiting;
}

void ABasicGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABasicGameMode::UpdateMatchState(EMatchState NewState)
{
	CurrentState = NewState;

	if (CurrentState == EMatchState::Waiting)
	{
		// 1. ���� ���� ������ �غ� (����ý��� Ȱ��)
		RoundTimer = StayTime;
		MonsterSubsystem->EndRound();
		++CurrentRound;
	}
	else if (CurrentState == EMatchState::Playing)
	{
		// 2. ��� ���� ���͵� �����
		if (MonsterSubsystem->MainSpawner)
		{
			MonsterSubsystem->StartRound(CurrentRound,40);
		}
		RoundTimer = PlayTime;
	}
}

void ABasicGameMode::UpdateTimerEverySecond()
{
	RoundTimer -= 1.0f;

	if (RoundTimer <= 0.0f)
	{
		if (CurrentState == EMatchState::Waiting) UpdateMatchState(EMatchState::Playing);
		else UpdateMatchState(EMatchState::Waiting);
	}
	OnTimerUpdated.Broadcast(RoundTimer);
}
