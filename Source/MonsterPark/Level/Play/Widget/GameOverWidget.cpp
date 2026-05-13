// Fill out your copyright notice in the Description page of Project Settings.


#include "GameOverWidget.h"
#include "Kismet/GameplayStatics.h"

void UGameOverWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Btn_Restart)
        Btn_Restart->OnClicked.AddDynamic(this, &UGameOverWidget::OnRestartClick);

    if (Btn_Quit)
        Btn_Quit->OnClicked.AddDynamic(this, &UGameOverWidget::OnQuitClicked);
}

void UGameOverWidget::OnRestartClick()
{
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}

void UGameOverWidget::OnQuitClicked()
{
    UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
