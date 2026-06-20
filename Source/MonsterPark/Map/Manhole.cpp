#include "Manhole.h"

AManhole::AManhole()
{
    PrimaryActorTick.bCanEverTick = true;

    PrimaryActorTick.bStartWithTickEnabled = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    Hinge = CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
    Hinge->SetupAttachment(Root);

    CoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoverMesh"));
    CoverMesh->SetupAttachment(Hinge);

    TargetRotation = FRotator::ZeroRotator;
}

void AManhole::OpenManhole()
{
    bIsOpen = true;
    TargetRotation = FRotator(55.f, 0.f, 0.f);

    SetActorTickEnabled(true);
}

void AManhole::CloseManhole()
{
    bIsOpen = false;

    TargetRotation = FRotator::ZeroRotator;

    SetActorTickEnabled(true);
}

void AManhole::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FRotator CurrentRot = Hinge->GetRelativeRotation();

    if (!CurrentRot.Equals(TargetRotation, 0.1f))
    {
        FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRotation, DeltaTime, 5.0f);
        Hinge->SetRelativeRotation(NewRot);
    }
    else
    {
        Hinge->SetRelativeRotation(TargetRotation); 
        SetActorTickEnabled(false); 
    }
}