// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnStartClicked);
	}

	if (GameOff)
	{
		GameOff->OnClicked.AddDynamic(this, &ULobbyWidget::OnGameOffClicked);
	}
}

void ULobbyWidget::OnStartClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName("LoadingLevel"));
}

void ULobbyWidget::OnGameOffClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
