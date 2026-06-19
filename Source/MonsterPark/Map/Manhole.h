// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Manhole.generated.h"

UCLASS()
class MONSTERPARK_API AManhole : public AActor
{
    GENERATED_BODY()

public:
    AManhole();
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Manhole")
    void OpenManhole();

    UFUNCTION(BlueprintCallable, Category = "Manhole")
    void CloseManhole();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* Hinge;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* CoverMesh;

    FRotator TargetRotation;

    bool bIsOpen = false;

};
