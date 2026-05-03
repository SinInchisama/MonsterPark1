// Fill out your copyright notice in the Description page of Project Settings.


#include"MonsterPark/Monster/AttackVisualActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

AAttackVisualActor::AAttackVisualActor()
{
    // 애니메이션 액터이므로 SkeletalMesh가 핵심입니다.
    MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // 공격 애니메이션은 보통 일회성이므로 Tick은 꺼두는 것이 성능에 좋습니다.
    PrimaryActorTick.bCanEverTick = false;
}

void AAttackVisualActor::PlayAttackAnimation()
{
    if (AttackMontage && MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Target Location Check"));
        UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
        if (AnimInstance && !AnimInstance->Montage_IsPlaying(AttackMontage))
        {
            AnimInstance->Montage_Play(AttackMontage);
        }
    }
}