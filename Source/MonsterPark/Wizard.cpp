// Fill out your copyright notice in the Description page of Project Settings.


#include "Wizard.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

void AWizard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
};

UAnimMontage* AWizard::GetDetectedMontage() const
{
    return WizardFullMontage;
}

void AWizard::PlayDetectedMontageIfNeeded()
{
    if (!bEnemyDetected) return;

    USkeletalMeshComponent* CharacterMesh = GetMesh();
    if (CharacterMesh)
    {
        if (UAnimInstance* AnimInst = CharacterMesh->GetAnimInstance())
        {
            if (WizardFullMontage && !AnimInst->Montage_IsPlaying(WizardFullMontage))
            {
                AnimInst->Montage_Play(WizardFullMontage);

                if (!bHasPlayedPassive)
                {
                    AnimInst->Montage_JumpToSection(FName("Passive"), WizardFullMontage);
                    bHasPlayedPassive = true;
                }
                else
                {
                    AnimInst->Montage_JumpToSection(FName("Attack"), WizardFullMontage);
                }
            }
        }
    }
}