// Fill out your copyright notice in the Description page of Project Settings.


#include "FogManager.h"
#include "Engine/Canvas.h"
#include "Kismet/GameplayStatics.h"
#include "CharacterBase.h"
#include "CanvasItem.h"
#include "MyBasicCharacter.h"

// Sets default values
AFogManager::AFogManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFogManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (FogRenderTarget)
	{
		FogRenderTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &AFogManager::UpdateFog);
	}
}

// Called every frame
void AFogManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (FogRenderTarget)
	{
		FogRenderTarget->UpdateResource();
	}
}

void AFogManager::UpdateFog(UCanvas* Canvas, int32 Width, int32 Height)
{
    if (!Canvas || !BrushMaterial) return;

    

    AMyBasicCharacter* PlayerChar = Cast<AMyBasicCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));


    if (PlayerChar)
    {
        // 2. 플레이어의 MySummonedHero 배열에 접근하여 순회합니다.
        for (ACharacterBase* Hero : PlayerChar->MySummonedHero)
        {
            if (Hero)
            {
                FVector Pos = Hero->GetActorLocation();


                // 맵 좌표 변환 (10000x10000 기준)
                float CanvasX = ((Pos.X / 25200.0f) + 0.5f) * Width;
                float CanvasY = ((Pos.Y / 25200.0f) + 0.5f) * Height;

                float SightRadius = 300.0f;

                // 3. 현재 소환된 영웅의 위치에 시야 그리기
                Canvas->K2_DrawMaterial(
                    BrushMaterial,
                    FVector2D(CanvasX - (SightRadius * 0.5f), CanvasY - (SightRadius * 0.5f)),
                    FVector2D(SightRadius, SightRadius),
                    FVector2D(0.f, 0.f),
                    FVector2D(1.f, 1.f)
                );
            }
        }
    }
}