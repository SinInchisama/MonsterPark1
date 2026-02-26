// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUserWidget.h"
#include "MyBasicCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BasicGameMode.h"
#include "AMyDetectionActor.h"

void UMyUserWidget::NativeConstruct()
{
	
	Btn_BuyHero_Array.Empty();		// BuyHero ��ư �迭 ä��� �κ�

	if (Btn_BuyHero0) Btn_BuyHero_Array.Add(Btn_BuyHero0);
	if (Btn_BuyHero1) Btn_BuyHero_Array.Add(Btn_BuyHero1);
	if (Btn_BuyHero2) Btn_BuyHero_Array.Add(Btn_BuyHero2);
	if (Btn_BuyHero3) Btn_BuyHero_Array.Add(Btn_BuyHero3);
	if (Btn_BuyHero4) Btn_BuyHero_Array.Add(Btn_BuyHero4);

	Text_HeroName_Array.Empty();
	if (Text_HeroName0) Text_HeroName_Array.Add(Text_HeroName0);
	if (Text_HeroName1) Text_HeroName_Array.Add(Text_HeroName1);
	if (Text_HeroName2) Text_HeroName_Array.Add(Text_HeroName2);
	if (Text_HeroName3) Text_HeroName_Array.Add(Text_HeroName3);
	if (Text_HeroName4) Text_HeroName_Array.Add(Text_HeroName4);

	Text_HeroPrice_Array.Empty();
	if (Text_HeroPrice0) Text_HeroPrice_Array.Add(Text_HeroPrice0);
	if (Text_HeroPrice1) Text_HeroPrice_Array.Add(Text_HeroPrice1);
	if (Text_HeroPrice2) Text_HeroPrice_Array.Add(Text_HeroPrice2);
	if (Text_HeroPrice3) Text_HeroPrice_Array.Add(Text_HeroPrice3);
	if (Text_HeroPrice4) Text_HeroPrice_Array.Add(Text_HeroPrice4);

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

	CachedGM = Cast<ABasicGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	ReFreshStore();
	ReFreshMoney();
}

void UMyUserWidget::Btn_LevelUp_Clicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AMyBasicCharacter* MyPlayer = Cast<AMyBasicCharacter>(PC->GetPawn());
		if(MyPlayer){
			if (MyPlayer->Get_PlayerMoney() >= 2) {
				MyPlayer->Set_PlayerMoney(-2);
				ReFreshMoney();
			}
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
			if (MyPlayer->Get_PlayerMoney() >= 2) {
				MyPlayer->Set_PlayerMoney(-2);
				ReFreshStore();
				ReFreshMoney();
			}
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
			ReFreshMoney();
			break;
		}
	}
}

void UMyUserWidget::ProcessHeroPurchase(int32 Btn_Num)
{
	APlayerController* PC = GetOwningPlayer();
	AMyBasicCharacter* MyPlayer = Cast<AMyBasicCharacter>(PC->GetPawn());
	
	int32 PriceInt = FCString::Atoi(*(Text_HeroPrice_Array[Btn_Num]->GetText().ToString()));

	if (MyPlayer->Get_PlayerMoney() >= PriceInt)
	{
		TSubclassOf<AAMyDetectionActor> SelectedClass = CurrentSlotClasses[Btn_Num];
		if (SelectedClass)
		{
			AAMyDetectionActor* DefaultHero = SelectedClass->GetDefaultObject<AAMyDetectionActor>();
			if (DefaultHero && CachedGM)
			{
				// ���� �� ���� ����
				CachedGM->SpawnHeroFromShop(SelectedClass, MyPlayer);
				MyPlayer->Set_PlayerMoney(-PriceInt);
				Btn_BuyHero_Array[Btn_Num]->SetIsEnabled(false);
				Text_HeroName_Array[Btn_Num]->SetText(FText::GetEmpty());
				Text_HeroPrice_Array[Btn_Num]->SetText(FText::GetEmpty());
			}
		}
	}
}

void UMyUserWidget::ReFreshStore()
{
	CurrentSlotClasses.Reset();

	for (int32 i = 0; i < Text_HeroName_Array.Num(); i++)
	{
		int32 RandomIdx = FMath::RandRange(0, CachedGM->MonsterClasses.Num() - 1);
		TSubclassOf<AActor> RandomClass = CachedGM->MonsterClasses[RandomIdx];

		// �θ� Ŭ����(AMyHeroBase)�� ĳ���� ������� ��ȯ�Ͽ� ����
		TSubclassOf<AAMyDetectionActor> HeroBaseClass = *RandomClass;
		CurrentSlotClasses.Add(HeroBaseClass);

		AAMyDetectionActor* CDO = HeroBaseClass->GetDefaultObject<AAMyDetectionActor>();
		if (CDO && Text_HeroName_Array.IsValidIndex(i))
		{
			Btn_BuyHero_Array[i]->SetIsEnabled(true);
			Text_HeroName_Array[i]->SetText(CDO->HeroDisplayName);
			int32 Price = CDO->HeroPrice;
			Text_HeroPrice_Array[i]->SetText(FText::AsNumber(Price));
		}
	}
}

void UMyUserWidget::ReFreshMoney()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AMyBasicCharacter* MyPlayer = Cast<AMyBasicCharacter>(PC->GetPawn());
		if (MyPlayer) {
			Money->SetText(FText::AsNumber(MyPlayer->Get_PlayerMoney()));
		}
	}
}
