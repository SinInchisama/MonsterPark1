// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterPark/Monster/AttackVisualActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "Animation/AnimInstance.h"

AAttackVisualActor::AAttackVisualActor()
{
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DummyRoot"));
	RootComponent = DummyRoot;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(DummyRoot);

	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComponent"));
	HealthBarComponent->SetupAttachment(DummyRoot);

	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f)); 
	HealthBarComponent->SetDrawSize(FVector2D(100.0f, 15.0f));

    PrimaryActorTick.bCanEverTick = false;
}

void AAttackVisualActor::PlayAttackAnimation()
{
	if (MeshComponent)
	{
		UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();

		if (AnimInstance && RandomAttackMontages.Num() > 0)
		{
			int32 RandomIndex = FMath::RandHelper(RandomAttackMontages.Num());
			UAnimMontage* SelectedMontage = RandomAttackMontages[RandomIndex];

			if (SelectedMontage && !AnimInstance->Montage_IsPlaying(SelectedMontage))
			{
				AnimInstance->Montage_Play(SelectedMontage);
			}
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

void AAttackVisualActor::UpdateHealthUI_Implementation(float HealthPercent)
{
}