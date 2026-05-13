#include "TeleportActor.h"
#include "Components/StaticMeshComponent.h"
#include "CharacterBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

ATeleportActor::ATeleportActor()
{
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    PortalLoopEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PortalLoopEffect"));
    PortalLoopEffect->SetupAttachment(RootComponent);

    TargetLocation = FVector(300.f, 0.f, 0.f);
}

void ATeleportActor::BeginPlay()
{
    Super::BeginPlay();

    if (MeshComponent)
    {
        MeshComponent->SetGenerateOverlapEvents(true);

        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

        MeshComponent->SetCollisionResponseToAllChannels(ECR_Overlap);

        MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ATeleportActor::OnMeshOverlap);
    }
}

void ATeleportActor::OnMeshOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        ACharacterBase* TargetHero = Cast<ACharacterBase>(OtherActor);

        if (TargetHero)
        {
            FHitResult HitResult;
            FVector Start = TargetLocation + FVector(0.f, 0.f, 500.f);
            FVector End = TargetLocation + FVector(0.f, 0.f, -500.f);

            FCollisionQueryParams Params;

            FVector FinalLocation = TargetLocation;

            if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, Params))
            {
                FinalLocation = HitResult.Location + FVector(0.f, 0.f, 110.f);
            }

            TargetHero->SetActorLocation(FinalLocation);

          /*  if (TeleportBurstEffect)
            {
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TeleportBurstEffect, FinalLocation);
            }*/

            TargetHero->SetIsOutside(true);
        }
    }
}