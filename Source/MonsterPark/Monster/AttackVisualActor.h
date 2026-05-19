// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AttackVisualActor.generated.h"

UCLASS()
class MONSTERPARK_API AAttackVisualActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAttackVisualActor();

void PlayAttackAnimation();
void PlayDeathAnimation();
protected:
    // 애니메이션을 위해 SkeletalMesh를 사용합니다.
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USkeletalMeshComponent* MeshComponent;

    // 에디터에서 공격 동작(몽타주)을 할당합니다.
    UPROPERTY(EditAnywhere, Category = "Animation")
    class UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    class UAnimMontage* DeathMontage;

};
