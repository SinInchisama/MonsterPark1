// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUserWidget.h"

void UMyUserWidget::NativeConstruct()
{
	if (Btn_LevelUp)
	{
		Btn_LevelUp->OnClicked.AddDynamic(this,&UMyUserWidget::Btn_LevelUp_Clicked);
	}
	if (Btn_Reload)
	{
		Btn_Reload->OnClicked.AddDynamic(this, &UMyUserWidget::Btn_Reload_Clicked);
	}
}

void UMyUserWidget::Btn_LevelUp_Clicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Level Up"));
}

void UMyUserWidget::Btn_Reload_Clicked()
{
}

void UMyUserWidget::Btn_BuyHero_Clicked()
{
}
