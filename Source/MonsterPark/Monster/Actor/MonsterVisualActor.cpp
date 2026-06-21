// Fill out your copyright notice in the Description page of Project Settings.

#include "MonsterVisualActor.h"
#include "Components/WidgetComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AMonsterVisualActor::AMonsterVisualActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DummyRoot"));
	RootComponent = DummyRoot;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(DummyRoot);

	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComponent"));
	HealthBarComponent->SetupAttachment(DummyRoot);

	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);

	HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f));
	HealthBarComponent->SetDrawSize(FVector2D(100.0f, 15.0f));
}

void AMonsterVisualActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMonsterVisualActor::UpdateHealthUI_Implementation(float CurrentHealth)
{
}