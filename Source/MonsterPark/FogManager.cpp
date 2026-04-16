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
        for (ACharacterBase* Hero : PlayerChar->MySummonedHero)
        {
            if (Hero)
            {
                FVector Pos = Hero->GetActorLocation();

   
                float CanvasX = ((Pos.X / 25200.0f) + 0.5f) * Width;
                float CanvasY = ((Pos.Y / 25200.0f) + 0.5f) * Height;

                float SightRadius = 100.0f;

                FVector2D ItemPos(CanvasX - (SightRadius * 0.5f), CanvasY - (SightRadius * 0.5f));
                FVector2D ItemSize(SightRadius, SightRadius);

                FCanvasTileItem TileItem(ItemPos, BrushMaterial->GetRenderProxy(), ItemSize);


                TileItem.BlendMode = ESimpleElementBlendMode::SE_BLEND_AlphaBlend;

      
                Canvas->DrawItem(TileItem);
            }
        }
    }
}