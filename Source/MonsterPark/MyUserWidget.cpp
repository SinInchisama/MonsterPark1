// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUserWidget.h"
#include "MyBasicCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BasicGameMode.h"
#include "Layout/Geometry.h"
#include "Input/Reply.h"
#include "Layout/SlateRotatedRect.h"

#include "PlaySubSystem.h"

#include "Components/Image.h"
#include "Engine/Texture2D.h"

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

	Img_Portrait_Array.Empty();
	Img_Portrait_Array.Add(Img_Portrait_0);
	Img_Portrait_Array.Add(Img_Portrait_1);
	Img_Portrait_Array.Add(Img_Portrait_2);
	Img_Portrait_Array.Add(Img_Portrait_3);
	Img_Portrait_Array.Add(Img_Portrait_4);

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

	AMyBasicCharacter* MyChar = Cast<AMyBasicCharacter>(GetOwningPlayerPawn());
	if (MyChar)
	{
		MyChar->OnLifeChanged.AddDynamic(this, &UMyUserWidget::UpdateLife);

		UpdateLife(MyChar->Get_PlayerLife());
	}

	if (CachedGM)
	{
		CachedGM->OnTimerUpdated.AddDynamic(this, &UMyUserWidget::UpdateTimer);

		UpdateTimer(20.f);
		ReFreshStore();
	}

	if (UWorld* World = GetWorld())
	{
		if (UPlaySubSystem* PlaySubsystem = World->GetSubsystem<UPlaySubSystem>())
		{
			PlaySubsystem->OnRoundChanged.AddDynamic(this, &UMyUserWidget::UpdateRoundText);

			UpdateRoundText(PlaySubsystem->CurrentRound);
		}
	}
	ReFreshMoney();
}

FReply UMyUserWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (Img_Minimap)
		{
			FGeometry MinimapGeometry = Img_Minimap->GetCachedGeometry();

			if (MinimapGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
			{
				FVector2D LocalPos = MinimapGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
				FVector2D MinimapSize = MinimapGeometry.GetLocalSize();

				float RatioX = LocalPos.X / MinimapSize.X;
				float RatioY = LocalPos.Y / MinimapSize.Y;

				float WorldX = (RatioX - 0.5f) * 25200.0f;
				float WorldY = (RatioY - 0.5f) * 25200.0f;

				APlayerController* PC = GetOwningPlayer();
				if (PC)
				{
					AMyBasicCharacter* MyPlayer = Cast<AMyBasicCharacter>(PC->GetPawn());
					if (MyPlayer)
					{
						FVector CurrentLoc = MyPlayer->GetActorLocation();

						MyPlayer->SetActorLocation(FVector(WorldX, WorldY, CurrentLoc.Z));
					}
				}

				return FReply::Handled();
			}
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
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
				if (MyPlayer->CheckLevelUp())
				{
					if (MyPlayer->PlayerLevelUp())
					{
						Btn_LevelUp->SetIsEnabled(false);
					}
				}
				else
				{
					MyPlayer->Set_PlayerExp(2);
				}
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
	if (!PC || !Text_HeroPrice_Array.IsValidIndex(Btn_Num) || !CurrentSlotClasses.IsValidIndex(Btn_Num))
	{
		return;
	}

	if (!Text_HeroPrice_Array[Btn_Num])
	{
		return;
	}

	AMyBasicCharacter* MyPlayer = Cast<AMyBasicCharacter>(PC->GetPawn());
	if (!MyPlayer)
	{
		return;
	}

	int32 PriceInt = FCString::Atoi(*(Text_HeroPrice_Array[Btn_Num]->GetText().ToString()));

	if (MyPlayer->Get_PlayerMoney() >= PriceInt)
	{
		TSubclassOf<ACharacterBase> SelectedClass = CurrentSlotClasses[Btn_Num];
		if (SelectedClass)
		{
			ACharacterBase* DefaultHero = SelectedClass->GetDefaultObject<ACharacterBase>();
			if (DefaultHero && CachedGM)
			{
				CachedGM->SpawnHeroFromShop(SelectedClass, MyPlayer);
				MyPlayer->Set_PlayerMoney(-PriceInt);
				if (Btn_BuyHero_Array.IsValidIndex(Btn_Num) && Btn_BuyHero_Array[Btn_Num])
				{
					Btn_BuyHero_Array[Btn_Num]->SetIsEnabled(false);
				}
				if (Text_HeroName_Array.IsValidIndex(Btn_Num) && Text_HeroName_Array[Btn_Num])
				{
					Text_HeroName_Array[Btn_Num]->SetText(FText::GetEmpty());
				}
				Text_HeroPrice_Array[Btn_Num]->SetText(FText::GetEmpty());
				if (Img_Portrait_Array.IsValidIndex(Btn_Num) && Img_Portrait_Array[Btn_Num])
				{
					Img_Portrait_Array[Btn_Num]->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.f));
				}
			}
		}
	}
}

void UMyUserWidget::ReFreshStore()
{
	AMyBasicCharacter* PC = Cast<AMyBasicCharacter>(GetOwningPlayerPawn());
	if (!CachedGM || !PC || Img_Portrait_Array.Num() == 0)
	{
		return;
	}

	CurrentSlotClasses.Reset();

	const int32 SlotCount = Text_HeroName_Array.Num();
	for (int32 i = 0; i < SlotCount; i++)
	{
		if (!Text_HeroName_Array.IsValidIndex(i) || !Text_HeroPrice_Array.IsValidIndex(i) ||
			!Btn_BuyHero_Array.IsValidIndex(i) || !Img_Portrait_Array.IsValidIndex(i))
		{
			continue;
		}

		TSubclassOf<ACharacterBase> HeroBaseClass = CachedGM->GetRandomHeroByChance(PC->CurrentLevelChance);
		CurrentSlotClasses.Add(HeroBaseClass);

		if (!HeroBaseClass)
		{
			Img_Portrait_Array[i]->SetBrushFromTexture(nullptr);
			continue;
		}

		ACharacterBase* CDO = HeroBaseClass->GetDefaultObject<ACharacterBase>();
		if (CDO)
		{
			if (Btn_BuyHero_Array[i])
			{
				Btn_BuyHero_Array[i]->SetIsEnabled(true);
			}
			Text_HeroName_Array[i]->SetText(CDO->UnitName);
			int32 Price = static_cast<int32>(CDO->DefaultCost);
			Text_HeroPrice_Array[i]->SetText(FText::AsNumber(Price));

			if (CDO->UnitPortrait)
			{
				Img_Portrait_Array[i]->SetBrushFromTexture(CDO->UnitPortrait);
			}
			else
			{
				Img_Portrait_Array[i]->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.f));
			}
		}
	}
}

void UMyUserWidget::ReFreshMoney()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		AMyBasicCharacter* MyPlayer = Cast<AMyBasicCharacter>(PC->GetPawn());
		if (MyPlayer && Money)
		{
			Money->SetText(FText::AsNumber(MyPlayer->Get_PlayerMoney()));
		}
	}
}

void UMyUserWidget::UpdateLife(int32 CurrentLife)
{
	RemainLife->SetText(FText::AsNumber(CurrentLife));
}

void UMyUserWidget::UpdateTimer(float Timer)
{
	int32 Seconds = FMath::FloorToInt(Timer);
	RemainTime-> SetText(FText::AsNumber(Seconds));

	ReFreshMoney();
}

void UMyUserWidget::UpdateRoundText(int32 NewRound)
{
	if (CurrentRound)
	{
		CurrentRound->SetText(FText::AsNumber(NewRound + 1));
	}
}


