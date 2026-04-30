// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "MyUserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"

AMyPlayerState::AMyPlayerState()
{
	CurrentLevelChance.Cost1  = 100.f;
}

void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyPlayerState, CurrentLevelChance);
	DOREPLIFETIME(AMyPlayerState, MyShopHeroes);
    DOREPLIFETIME(AMyPlayerState, Money);
}

void AMyPlayerState::OnRep_ShopHeroes()
{
	if (MyStoreWidget)
	{
		MyStoreWidget->ReFreshStore();
	}
}

void AMyPlayerState::Client_NotifyShopRefreshed_Implementation(const TArray<TSubclassOf<ACharacterBase>>& NewHeroes)
{
    if (!MyStoreWidget)
    {
        TArray<UUserWidget*> FoundWidgets;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UMyUserWidget::StaticClass());
        if (FoundWidgets.Num() > 0)
        {
            MyStoreWidget = Cast<UMyUserWidget>(FoundWidgets[0]);
        }
    }

    MyShopHeroes = NewHeroes;

    if (MyStoreWidget)
    {
        MyStoreWidget->ReFreshStore();
    }
}
