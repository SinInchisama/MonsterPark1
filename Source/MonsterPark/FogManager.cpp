// Fill out your copyright notice in the Description page of Project Settings.


#include "FogManager.h"
#include "Engine/Canvas.h"
#include "Kismet/GameplayStatics.h"
#include "CharacterBase.h"
#include "CanvasItem.h"
#include "Engine/EngineTypes.h"
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

    Canvas->K2_DrawBox(FVector2D(0, 0), FVector2D(Width, Height), 0.0f, FLinearColor::Black);

    AMyBasicCharacter* PlayerChar = Cast<AMyBasicCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));


    if (PlayerChar)
    {
        // 2. 플레이어의 MySummonedHero 배열에 접근하여 순회합니다.
        for (ACharacterBase* Hero : PlayerChar->MySummonedHero)
        {
            if (Hero)
            {
                FVector Pos = Hero->GetActorLocation();

                // 1. 좌표 변환 (기존과 동일)
                float CanvasX = ((Pos.X / 25200.0f) + 0.5f) * Width;
                float CanvasY = ((Pos.Y / 25200.0f) + 0.5f) * Height;

                float SightRadius = 100.0f;

                // 2. TileItem 생성 (K2_DrawMaterial 대신 직접 아이템 생성)
                // 위치는 좌상단 기준이므로 중심점 보정을 위해 (SightRadius * 0.5f)를 뺍니다.
                FVector2D ItemPos(CanvasX - (SightRadius * 0.5f), CanvasY - (SightRadius * 0.5f));
                FVector2D ItemSize(SightRadius, SightRadius);

                // 3. 캔버스 타일 아이템 설정
                FCanvasTileItem TileItem(ItemPos, BrushMaterial->GetRenderProxy(), ItemSize);

                // [핵심] 블렌드 모드를 Max(최댓값)로 설정
                // 이렇게 하면 픽셀 값이 더해지지 않고, 기존 값과 새 값 중 더 큰 값(밝은 값)만 남습니다.
                TileItem.BlendMode = ESimpleElementBlendMode::SE_BLEND_AlphaBlend;

                // 4. 그리기 실행
                Canvas->DrawItem(TileItem);
            }
        }
    }
}