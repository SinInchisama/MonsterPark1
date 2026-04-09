#include "TeleportActor.h"
#include "Components/StaticMeshComponent.h"
#include "CharacterBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

ATeleportActor::ATeleportActor()
{
    // 1. 메시 컴포넌트 생성 및 루트 설정
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    PortalLoopEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PortalLoopEffect"));
    PortalLoopEffect->SetupAttachment(RootComponent);

    // 2. 기본 목적지 설정 (상대 좌표)
    TargetLocation = FVector(300.f, 0.f, 0.f);
}

void ATeleportActor::BeginPlay()
{
    Super::BeginPlay();

    // 3. 콜리전 설정 (생성자보다 BeginPlay가 안전함)
    if (MeshComponent)
    {
        // 겹침 이벤트 발생 활성화
        MeshComponent->SetGenerateOverlapEvents(true);

        // 쿼리(Overlap/Raycast)는 수행하고, 물리(Block)는 하지 않음
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

        // 모든 채널에 대해 Overlap으로 설정 (가장 확실한 방법)
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Overlap);

        // 이벤트 바인딩
        MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ATeleportActor::OnMeshOverlap);
    }
}

void ATeleportActor::OnMeshOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        // ACharacterBase인지 확인
        ACharacterBase* TargetHero = Cast<ACharacterBase>(OtherActor);

        if (TargetHero)
        {
            // 좌표 변환 및 이동
            TargetHero->SetActorLocation(TargetLocation);

            if (TeleportBurstEffect)
            {
                // 특정 위치에 이펙트 생성 (Spawn System at Location)
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TeleportBurstEffect, TargetLocation);
            }
        }
    }
}