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
	bIsDrawing = false;
	StartDrawPoint = FVector2D::ZeroVector;
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

void AGame_HUD::DrawHUD()
{
	Super::DrawHUD();

	if (bIsDrawing)
	{
		float CurrentMouseX, CurrentMouseY;

		if (GetOwningPlayerController())
		{
			GetOwningPlayerController()->GetMousePosition(CurrentMouseX, CurrentMouseY);

			FLinearColor LineColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
			float LineThickness = 2.0f; 
			DrawLine(StartDrawPoint.X, StartDrawPoint.Y, CurrentMouseX, StartDrawPoint.Y, LineColor, LineThickness);
			DrawLine(StartDrawPoint.X, CurrentMouseY, CurrentMouseX, CurrentMouseY, LineColor, LineThickness);
			DrawLine(StartDrawPoint.X, StartDrawPoint.Y, StartDrawPoint.X, CurrentMouseY, LineColor, LineThickness);
			DrawLine(CurrentMouseX, StartDrawPoint.Y, CurrentMouseX, CurrentMouseY, LineColor, LineThickness);
		}
	}
}
