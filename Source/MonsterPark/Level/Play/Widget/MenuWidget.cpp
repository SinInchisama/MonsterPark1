// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"
#include "Kismet/GameplayStatics.h"

void UMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Btn_Restart)
        Btn_Restart->OnClicked.AddDynamic(this, &UMenuWidget::OnRestartClick);

    if (Btn_Quit)
        Btn_Quit->OnClicked.AddDynamic(this, &UMenuWidget::OnQuitClicked);

    if(Btn_StopRestart)
        Btn_StopRestart->OnClicked.AddDynamic(this, &UMenuWidget::OnStopRestart);
}

void UMenuWidget::OnRestartClick()
{
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}

void UMenuWidget::OnQuitClicked()
{
    UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

void UMenuWidget::OnStopRestart()
{
    GameStop = !GameStop;

    UGameplayStatics::SetGamePaused(GetWorld(), GameStop);
}
