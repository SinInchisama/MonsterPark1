// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeleportActor.generated.h"

UCLASS()
class MONSTERPARK_API ATeleportActor : public AActor
{
    GENERATED_BODY()

public:
    ATeleportActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* MeshComponent;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport", meta = (MakeEditWidget = true, DisplayName = "¿Ãµø"))
    FVector TargetLocation;

    UPROPERTY(VisibleAnywhere)
    class UNiagaraComponent* PortalLoopEffect;

    UPROPERTY(EditAnywhere, Category = "Effects")
    TObjectPtr<class UNiagaraSystem> TeleportBurstEffect;

    UFUNCTION()
    void OnMeshOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};