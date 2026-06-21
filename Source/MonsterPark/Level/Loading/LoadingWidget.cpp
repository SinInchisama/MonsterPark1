// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterPark/Level/Loading/LoadingWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "LoadingGameMode.h"

void ULoadingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bLevelTransitionStarted) return;

    ElapsedTime += InDeltaTime;
    float CurrentPercent = FMath::Clamp(ElapsedTime / TotalLoadingTime, 0.0f, 1.0f);

    if (LoadingBar)
    {
        LoadingBar->SetPercent(CurrentPercent);
    }

    if (CurrentPercent >= 1.0f)
    {
        bLevelTransitionStarted = true;

        UGameplayStatics::OpenLevel(GetWorld(), FName("MonsterPARK"));
    }
}