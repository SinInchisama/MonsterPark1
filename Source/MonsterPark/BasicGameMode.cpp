// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicGameMode.h"
#include "Game_HUD.h"
#include "MyBasicCharacter.h"
#include "PlaySubSystem.h"
#include "Engine/Engine.h"

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

	TryStartTimer();

	/*if (MonsterSubsystem)
	{
		if (MonsterSubsystem->MainSpawner)
		{
			RoundTimer = StayTime;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &ABasicGameMode::UpdateTimerEverySecond, 1.0f, true);

			OnTimerUpdated.Broadcast(RoundTimer);
		}
	}*/

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AMyBasicCharacter* MyChar = Cast<AMyBasicCharacter>(PC->GetPawn());
	if (MyChar)
	{
		MyChar->Mixtured.AddDynamic(this, &ABasicGameMode::Mixture);
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
		// 1. ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½Øºï¿½ (ï¿½ï¿½ï¿½ï¿½Ã½ï¿½ï¿½ï¿½ È°ï¿½ï¿½)
		RoundTimer = StayTime;
		MonsterSubsystem->EndRound();
		++CurrentRound;
	}
	else if (CurrentState == EMatchState::Playing)
	{
		// 2. ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½Íµï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½
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

void ABasicGameMode::Mixture(int32 cost)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AMyBasicCharacter* MyChar = Cast<AMyBasicCharacter>(PC->GetPawn());

	UE_LOG(LogTemp, Log, TEXT("MainSpawner!@@@@@@@@@@@@@@@@@@"));

	int32 RandomIdx = FMath::RandRange(0, MonsterTwoCoinClasses.Num() - 1);
	TSubclassOf<ACharacterBase> HeroBaseClass = MonsterTwoCoinClasses[RandomIdx];

	SpawnHeroFromShop(HeroBaseClass, MyChar);
}

void ABasicGameMode::TryStartTimer()
{
	if (!MonsterSubsystem)
	{
		MonsterSubsystem = GetWorld()->GetSubsystem<UPlaySubSystem>();
		return;
	}

	if (MonsterSubsystem && MonsterSubsystem->MainSpawner)
	{
		RoundTimer = StayTime;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ABasicGameMode::UpdateTimerEverySecond, 1.0f, true);
		OnTimerUpdated.Broadcast(RoundTimer);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,                 // °íÀ¯ Å° (Key)
				5.f,                // Áö¼Ó ½Ã°£ (Seconds)
				FColor::Cyan,       // »ö»ó (Color)
				TEXT("spspspspspspspsp") // ³»¿ë (Text)
			);
		}

	}
	else
	{
		GetWorldTimerManager().SetTimer(SpawnerCheckHandle, this, &ABasicGameMode::TryStartTimer, 0.1f, false);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,                 // °íÀ¯ Å° (Key)
				5.f,                // Áö¼Ó ½Ã°£ (Seconds)
				FColor::Cyan,       // »ö»ó (Color)
				TEXT("asmaksmakw") // ³»¿ë (Text)
			);
		}
	}
}