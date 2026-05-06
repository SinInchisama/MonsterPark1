// Fill out your copyright notice in the Description page of Project Settings.

#include "LoadingGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void ALoadingGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (LoadingWidgetClass)
    {
        CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), LoadingWidgetClass);
        CurrentWidget->AddToViewport();
    }

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &ALoadingGameMode::StartAsyncLevelLoad, 0.1f, false);
}

void ALoadingGameMode::StartAsyncLevelLoad()
{
    FString LevelPath = TEXT("/Game/Level/MonsterPARK");

    LoadPackageAsync(LevelPath, FLoadPackageAsyncDelegate::CreateLambda([this, LevelPath](const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result)
        {
            if (Result == EAsyncLoadingResult::Succeeded)
            {
                UGameplayStatics::OpenLevel(this, FName(*LevelPath));
            }
        }), 0, PKG_ContainsMap);
}

float ALoadingGameMode::GetLoadingProgress()
{
    ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(this, FName("MonsterPARK"));

    if (StreamingLevel)
    {
        if (StreamingLevel->IsLevelLoaded()) return 1.0f;
        return 0.5f;
    }
    return 0.0f;
}

void ALoadingGameMode::OnLevelLoaded()
{
    UE_LOG(LogTemp, Log, TEXT("Level Load Complete!"));
}