// Fill out your copyright notice in the Description page of Project Settings.


#include"MonsterPark/Monster/AttackVisualActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

AAttackVisualActor::AAttackVisualActor()
{
    MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    PrimaryActorTick.bCanEverTick = false;
}

void AAttackVisualActor::PlayAttackAnimation()
{
    if (AttackMontage && MeshComponent)
    {
        UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
        if (AnimInstance && !AnimInstance->Montage_IsPlaying(AttackMontage))
        {
            AnimInstance->Montage_Play(AttackMontage);
        }
    }
}

void AAttackVisualActor::PlayDeathAnimation()
{
    if (DeathMontage && MeshComponent)
    {
        UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
        if (AnimInstance && !AnimInstance->Montage_IsPlaying(DeathMontage))
        {
            AnimInstance->Montage_Play(DeathMontage);
        }
    }
}