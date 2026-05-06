// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterPark/Level/Loading/LoadingWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "LoadingGameMode.h"

void ULoadingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 엔진 내부의 비동기 로딩 퍼센트를 가져옵니다 (0 ~ 100)
    float ActualProgress = GetAsyncLoadPercentage(FName("/Game/Level/MonsterPARK"));

    if (ActualProgress >= 0.0f)
    {
        // 0.0 ~ 1.0 범위로 변환
        float TargetPercent = ActualProgress / 100.0f;

        // ProgressBar에 부드럽게 반영 (보간)
        LoadingBar->SetPercent(TargetPercent);
    }
}