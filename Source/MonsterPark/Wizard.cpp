// Fill out your copyright notice in the Description page of Project Settings.


#include "Wizard.h"

void AWizard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
};

UAnimMontage* AWizard::GetDetectedMontage() const
{
    // 부모 클래스의 PlayDetectedMontageIfNeeded()가 이 몽타주를 사용하게 됩니다.
    return WizardFullMontage;
}

void AWizard::PlayDetectedMontageIfNeeded()
{
    if (!bEnemyDetected) return;

    if (USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>())
    {
        if (UAnimInstance* AnimInst = Mesh->GetAnimInstance())
        {
            if (WizardFullMontage && !AnimInst->Montage_IsPlaying(WizardFullMontage))
            {
                // 1. 몽타주를 일단 실행
                AnimInst->Montage_Play(WizardFullMontage);

                if (!bHasPlayedPassive)
                {
                    // 처음 감지 시: Passive 섹션부터 재생 (기본값이면 생략 가능)
                    AnimInst->Montage_JumpToSection(FName("Passive"), WizardFullMontage);
                    bHasPlayedPassive = true; // 이제 패시브 실행됨을 기록
                    UE_LOG(LogTemp, Log, TEXT("Wizard: Playing Passive for the first time."));
                }
                else
                {
                    // 두 번째 감지부터: 바로 Attack 섹션으로 점프
                    AnimInst->Montage_JumpToSection(FName("Attack"), WizardFullMontage);
                    UE_LOG(LogTemp, Log, TEXT("Wizard: Playing Attack section."));
                }
            }
        }
    }
}