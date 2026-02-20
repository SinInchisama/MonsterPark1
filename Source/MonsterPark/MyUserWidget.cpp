// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUserWidget.h"
#include "MyBasicCharacter.h"

void UMyUserWidget::NativeConstruct()
{
	
	Btn_BuyHero_Array.Empty();		// BuyHero 버튼 배열 채우는 부분

	if (Btn_BuyHero1) Btn_BuyHero_Array.Add(Btn_BuyHero1);
	if (Btn_BuyHero2) Btn_BuyHero_Array.Add(Btn_BuyHero2);
	if (Btn_BuyHero3) Btn_BuyHero_Array.Add(Btn_BuyHero3);
	if (Btn_BuyHero4) Btn_BuyHero_Array.Add(Btn_BuyHero4);
	if (Btn_BuyHero5) Btn_BuyHero_Array.Add(Btn_BuyHero5);

	if (Btn_LevelUp)
	{
		Btn_LevelUp->OnClicked.AddDynamic(this,&UMyUserWidget::Btn_LevelUp_Clicked);
	}
	if (Btn_Reload)
	{
		Btn_Reload->OnClicked.AddDynamic(this, &UMyUserWidget::Btn_Reload_Clicked);
	}

	for (UButton* Btn : Btn_BuyHero_Array)
	{
		Btn->OnClicked.AddDynamic(this, &UMyUserWidget::Btn_BuyHero_Clicked);
	}
}

void UMyUserWidget::Btn_LevelUp_Clicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AMyBasicCharacter* MyPlayer = Cast<AMyBasicCharacter>(PC->GetPawn());
		if(MyPlayer){
			MyPlayer->Set_PlayerMoney(-2);

			UE_LOG(LogTemp, Warning, TEXT("Level Up, %i"), MyPlayer->Get_PlayerMoney());
		}
	}
}

void UMyUserWidget::Btn_Reload_Clicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AMyBasicCharacter* MyPlayer = Cast<AMyBasicCharacter>(PC->GetPawn());
		if (MyPlayer) {
			MyPlayer->Set_PlayerMoney(-4);

			UE_LOG(LogTemp, Warning, TEXT("Reload, %i"), MyPlayer->Get_PlayerMoney());
		}
	}
}

void UMyUserWidget::Btn_BuyHero_Clicked()
{
	for (int32 i = 0; i < Btn_BuyHero_Array.Num(); i++)
	{
		if (Btn_BuyHero_Array[i] && Btn_BuyHero_Array[i]->HasUserFocus(GetOwningPlayer()))
		{
			ProcessHeroPurchase(i);
			break;
		}
	}
}

void UMyUserWidget::ProcessHeroPurchase(int32 Btn_Num)
{
	UE_LOG(LogTemp, Warning, TEXT("Selected Hero Index: %d"), Btn_Num);
}