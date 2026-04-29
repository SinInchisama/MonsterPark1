// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicGameMode.h"
#include  "Game_HUD.h"
#include "MyBasicCharacter.h"
#include "PlaySubSystem.h"
#include "Engine/Engine.h"
#include "PlayState.h"
#include "MyPlayerState.h"
ABasicGameMode::ABasicGameMode()
{
	HUDClass = AGame_HUD::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
}

void ABasicGameMode::SpawnHeroFromShop(TSubclassOf<ACharacterBase> HeroClass, ACharacter* PlayerChar)
{
	if (!HeroClass) return;

	FVector SpawnLocation(-1500.0f,1500.0f, 110.0f);
	FRotator SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters SpawnParams;

	ACharacterBase* NewHero = GetWorld()->SpawnActor<ACharacterBase>(HeroClass, SpawnLocation, SpawnRotation, SpawnParams);

	AMyBasicCharacter* MyChar = Cast<AMyBasicCharacter>(PlayerChar);
	if (MyChar)
	{
		MyChar->SetSummonedActor(NewHero);
	}
}

void ABasicGameMode::OnLevelUp(FHeroChanceRow& CurrentChane,int Level)
{
	if (!HeroChanceTable)
	{
		return;
	}

	FHeroChanceRow* FoundRow = HeroChanceTable->FindRow<FHeroChanceRow>(FName(*FString::FromInt(Level)), "");

	if (HeroChanceTable)
	{
		CurrentChane =  *FoundRow;
	}
}

TSubclassOf<ACharacterBase> ABasicGameMode::GetRandomHeroByChance(const FHeroChanceRow& ChanceData)
{
	float RandVal = FMath::FRandRange(0.0f, 100.0f);

	if (RandVal <= ChanceData.Cost1)
	{
		if (MonsterOneCoinClasses.Num() > 0)
			return MonsterOneCoinClasses[FMath::RandRange(0, MonsterOneCoinClasses.Num() - 1)];
	}
	else if (RandVal <= (ChanceData.Cost1 + ChanceData.Cost2))
	{
		if (MonsterTwoCoinClasses.Num() > 0)
			return MonsterTwoCoinClasses[FMath::RandRange(0, MonsterTwoCoinClasses.Num() - 1)];
	}
	else
	{
		if (MonsterThreeCoinClasses.Num() > 0)
			return MonsterThreeCoinClasses[FMath::RandRange(0, MonsterThreeCoinClasses.Num() - 1)];
	}

	return (MonsterOneCoinClasses.Num() > 0) ? MonsterOneCoinClasses[0] : nullptr;
}

void ABasicGameMode::RefreshIndividualShop(APlayerController* PC)
{
	AMyPlayerState* PS = PC->GetPlayerState<AMyPlayerState>();
	if (PS)
	{
		TArray<TSubclassOf<ACharacterBase>>  TempHeroes;
		PS->MyShopHeroes.Empty(5);
		for (int32 i = 0; i < 5; i++)
		{
			TempHeroes.Add(GetRandomHeroByChance(PS->CurrentLevelChance));
		}
		PS->MyShopHeroes = TempHeroes;

		PS->Client_NotifyShopRefreshed(TempHeroes);
	}
}


void ABasicGameMode::BeginPlay()
{
	Super::BeginPlay();

	TryStartTimer();

	CurrentState = EMatchState::Waiting;
}

void ABasicGameMode::PostLogin(APlayerController* PC)
{
	Super::PostLogin(PC);

	if (PC)
	{
		AMyBasicCharacter* MyChar = Cast<AMyBasicCharacter>(PC->GetPawn());
		if (MyChar)
		{
			MyChar->Mixtured.AddDynamic(this, &ABasicGameMode::Mixture);
			AMyPlayerState* PS = PC->GetPlayerState<AMyPlayerState>();
			GetRandomHeroByChance(PS->CurrentLevelChance);
		}
	}
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
		RoundTimer = StayTime;
		MonsterSubsystem->EndRound();

	}
	else if (CurrentState == EMatchState::Playing)
	{
		if (MonsterSubsystem->MainSpawner)
		{
			MonsterSubsystem->StartRound(0,40);
		}
		RoundTimer = PlayTime;
	}
}

void ABasicGameMode::UpdateTimerEverySecond()
{
	RoundTimer -= 1.0f;

	APlayState* PS = GetGameState<APlayState>();
	if (PS)
	{
		PS->RemainingTime = FMath::Max(0, FMath::FloorToInt(RoundTimer));
	}

	if (RoundTimer <= 0.0f)
	{
		if (CurrentState == EMatchState::Waiting) UpdateMatchState(EMatchState::Playing);
		else {
			UpdateMatchState(EMatchState::Waiting);
			PS->CurrentRound += 1;
		}
	}
}

void ABasicGameMode::Mixture(int32 cost)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AMyBasicCharacter* MyChar = Cast<AMyBasicCharacter>(PC->GetPawn());

	int32 RandomIdx = FMath::RandRange(0, MonsterTwoCoinClasses.Num() - 1);
	TSubclassOf<ACharacterBase> HeroBaseClass = MonsterTwoCoinClasses[RandomIdx];

	SpawnHeroFromShop(HeroBaseClass, MyChar);
}

void ABasicGameMode::TryStartTimer()
{
	if (!MonsterSubsystem)
	{
		MonsterSubsystem = GetWorld()->GetSubsystem<UPlaySubSystem>();
		GetWorldTimerManager().SetTimer(SpawnerCheckHandle, this, &ABasicGameMode::TryStartTimer, 0.1f, false);
		return;
	}

	if (MonsterSubsystem && MonsterSubsystem->MainSpawner)
	{
		RoundTimer = StayTime;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ABasicGameMode::UpdateTimerEverySecond, 1.0f, true);
	}
	else
	{
		GetWorldTimerManager().SetTimer(SpawnerCheckHandle, this, &ABasicGameMode::TryStartTimer, 0.1f, false);
	}
}