// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingWidget.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API ULoadingWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* LoadingBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* LoadingText;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	float TargetProgress = 0.0f;
	float CurrentDisplayProgress = 0.0f;

	float TotalLoadingTime = 2.0f;

	float ElapsedTime = 0.0f;

	bool bLevelTransitionStarted = false;
};
