// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicGameMode.h"
#include  "Game_HUD.h"
#include "MyBasicCharacter.h"
#include "PlaySubSystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

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

void ABasicGameMode::GameOver(bool bVictory)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && GameOverWidgetClass)
	{
		UUserWidget* GameOverUI = CreateWidget<UUserWidget>(GetWorld(), GameOverWidgetClass);
		if (GameOverUI)
		{
			GameOverUI->AddToViewport();

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(GameOverUI->TakeWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}

		UGameplayStatics::SetGamePaused(GetWorld(), true);
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


void ABasicGameMode::ToggleMenuUI()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	if (CurrentMenuWidget && CurrentMenuWidget->IsInViewport())
	{
		CurrentMenuWidget->RemoveFromParent();

		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		PC->SetInputMode(InputMode);
	}
	else if (MenuWidgetClass)
	{
		if (!CurrentMenuWidget)
		{
			CurrentMenuWidget = CreateWidget<UUserWidget>(GetWorld(), MenuWidgetClass);
		}

		if (CurrentMenuWidget)
		{
			CurrentMenuWidget->AddToViewport();

			FInputModeGameAndUI InputMode; 
			InputMode.SetWidgetToFocus(CurrentMenuWidget->TakeWidget());
			PC->SetInputMode(InputMode);
		}
	}
}

void ABasicGameMode::BeginPlay()
{
	Super::BeginPlay();

	TryStartTimer();

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
		// 1. ���� ���� ������ �غ� (����ý��� Ȱ��)
		RoundTimer = StayTime;
		MonsterSubsystem->EndRound();
	}
	else if (CurrentState == EMatchState::Playing)
	{
		// 2. ��� ���� ���͵� �����
		if (MonsterSubsystem->MainSpawners.Num() > 0)
		{
			MonsterSubsystem->StartRound(0,40);
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

	if (MonsterSubsystem->MainSpawners.Num() > 0)
	{
		RoundTimer = StayTime;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ABasicGameMode::UpdateTimerEverySecond, 1.0f, true);
		OnTimerUpdated.Broadcast(RoundTimer);
	}
	else
	{
		GetWorldTimerManager().SetTimer(SpawnerCheckHandle, this, &ABasicGameMode::TryStartTimer, 0.1f, false);
	}
}