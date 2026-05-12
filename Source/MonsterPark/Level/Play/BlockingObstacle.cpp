// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockingObstacle.h"
#include "MonsterPark/PlaySubSystem.h"

// Sets default values
ABlockingObstacle::ABlockingObstacle()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    GridCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("GridCollisionBox"));
    GridCollisionBox->SetupAttachment(RootComponent);

    GridCollisionBox->SetHiddenInGame(true);
    GridCollisionBox->SetCollisionProfileName(TEXT("NoCollision")); 

}

// Called when the game starts or when spawned
void ABlockingObstacle::BeginPlay()
{
    Super::BeginPlay();

    if (UPlaySubSystem* PlaySubsystem = GetWorld()->GetSubsystem<UPlaySubSystem>())
    {
        RegisteredKeys = GetAffectedGridKeys();
        PlaySubsystem->RegisterObstacle(RegisteredKeys);
    }
	
}

TArray<FIntVector> ABlockingObstacle::GetAffectedGridKeys() const
{
    TArray<FIntVector> Keys;
    UPlaySubSystem* PlaySubsystem = GetWorld()->GetSubsystem<UPlaySubSystem>();
    if (!PlaySubsystem) return Keys;

    // 1. 박스 컴포넌트의 실제 월드 경계(Bounds)를 가져옵니다.
    // CalcBounds는 스케일과 회전이 모두 적용된 박스의 최소/최대 좌표를 반환합니다.
    FBox SphereBox = GridCollisionBox->CalcBounds(GridCollisionBox->GetComponentTransform()).GetBox();

    FVector Min = SphereBox.Min;
    FVector Max = SphereBox.Max;

    // 2. GridSize 간격으로 루프를 돌며 포함되는 모든 격자 키 추출
    float Step = 100.f;

    // 중복을 방지하기 위해 TSet을 사용한 뒤 TArray로 변환하는 것이 효율적입니다.
    TSet<FIntVector> KeySet;

    // 박스의 최소 지점부터 최대 지점까지 격자 크기만큼 점프하며 좌표를 샘플링합니다.
    for (float x = Min.X; x <= Max.X + Step; x += Step)
    {
        for (float y = Min.Y; y <= Max.Y + Step; y += Step)
        {
            // 루프 제어: Max 값을 넘어가면 Max 값으로 고정해서 마지막 칸을 찍어줍니다.
            float TargetX = FMath::Min(x, Max.X);
            float TargetY = FMath::Min(y, Max.Y);

            KeySet.Add(PlaySubsystem->PosToGrid(FVector(TargetX, TargetY, 0)));
        }
    }

    return KeySet.Array();
}