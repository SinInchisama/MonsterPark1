// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GameOverWidget.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERPARK_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Restart;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Quit;

    UFUNCTION()
    void OnRestartClick();

    UFUNCTION()
    void OnQuitClicked();
};
