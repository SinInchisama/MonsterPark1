// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUserWidget.h"
#include "MyBasicCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BasicGameMode.h"
#include "AMyDetectionActor.h"

void UMyUserWidget::NativeConstruct()
{
	
	Btn_BuyHero_Array.Empty();		// BuyHero 버튼 배열 채우는 부분

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

void UMyUserWidget::ReFreshStore()
{
	for (int32 i = 0; i < Text_HeroName_Array.Num(); i++)
	{
		int32 RandomIdx = FMath::RandRange(0, CachedGM->MonsterClasses.Num() - 1);
		TSubclassOf<AActor> HeroClass = CachedGM->MonsterClasses[RandomIdx];
		AAMyDetectionActor* DefaultHero = Cast<AAMyDetectionActor>(HeroClass->GetDefaultObject());
		if (DefaultHero && Text_HeroName_Array.IsValidIndex(i))
		{
			Text_HeroName_Array[i]->SetText(DefaultHero->HeroDisplayName);
			int32 Price = DefaultHero->HeroPrice;
			Text_HeroPrice_Array[i]->SetText(FText::AsNumber(Price));
		}
	}
}