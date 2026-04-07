// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "FogManager.generated.h"

UCLASS()
class MONSTERPARK_API AFogManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFogManager();

	UPROPERTY(EditAnywhere, Category = "Fog")
	UCanvasRenderTarget2D* FogRenderTarget;

	UPROPERTY(EditAnywhere, Category = "Fog")
	UMaterialInterface* BrushMaterial;

	UPROPERTY(EditAnywhere, Category = "Fog")
	float MapSize = 10000.0f;

	UFUNCTION()
	void UpdateFog(UCanvas* Canvas, int32 Width, int32 Height);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
