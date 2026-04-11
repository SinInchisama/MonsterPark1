// Fill out your copyright notice in the Description page of Project Settings.


#include "TreasureChest.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "MonsterPark/CharacterBase.h"

#include "MonsterPark/MyBasicCharacter.h"

// Sets default values
ATreasureChest::ATreasureChest()
{
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    CollisionBox->SetupAttachment(RootComponent);
    CollisionBox->SetCollisionProfileName(TEXT("Trigger"));

    CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ATreasureChest::OnOverlapBegin);
}

void ATreasureChest::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ACharacterBase* Hero = Cast<ACharacterBase>(OtherActor))
    {
        if (DeathEffect)
        {
            FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 40.0f);

            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                DeathEffect,
                SpawnLocation,
                GetActorRotation() 
            );
        }

        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            AMyBasicCharacter* PlayerChar = Cast<AMyBasicCharacter>(PC->GetPawn());
            if (PlayerChar)
            {
                PlayerChar->Set_PlayerMoney(10);
            }
        }
        Destroy();
    }
}