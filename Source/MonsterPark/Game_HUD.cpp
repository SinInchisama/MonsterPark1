// Fill out your copyright notice in the Description page of Project Settings.


#include "Game_HUD.h"
#include "UObject/ConstructorHelpers.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

AGame_HUD::AGame_HUD()
{

	static ConstructorHelpers::FClassFinder<UUserWidget>ui(TEXT("/Game/UI/Play/Store.Store_C"));

	if (ui.Succeeded())
	{
		uiBPClass = ui.Class;
	}
}

void AGame_HUD::BeginPlay()
{
	Super::BeginPlay();

	//UE_LOG(LogTemp, Warning, TEXT("HUD BeginPlay Started!"));

	if (uiBPClass)
	{
		uiWidget = CreateWidget<UUserWidget>(GetWorld(),uiBPClass);
			if (uiWidget)
			{
				uiWidget->AddToViewport();
			}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("uiBPClass is NULL! Check your Constructor path."));
	}
}
