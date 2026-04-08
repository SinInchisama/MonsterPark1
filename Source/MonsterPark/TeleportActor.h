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

    // 박스 대신 스태틱 메시를 루트로 사용
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    // 에디터 뷰포트에서 조절 가능한 목적지 위젯
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport", meta = (MakeEditWidget = true, DisplayName = "이동"))
    FVector TargetLocation;

    UFUNCTION()
    void OnMeshOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};