// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;

	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;

	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false); // 클릭 중에도 커서 숨기지 않음

	SetInputMode(InputMode);
}

void AMyPlayerController::Server_SetPawnLocation_Implementation(FVector NewLocation)
{
	APawn* MyPawn = GetPawn();
	if (MyPawn)
	{
		MyPawn->SetActorLocation(NewLocation);
	}
}
