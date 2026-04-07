// Fill out your copyright notice in the Description page of Project Settings.


#include "Fighter.h"

void AFighter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
};

UAnimMontage* AFighter::GetDetectedMontage() const
{
    // 부모 클래스의 PlayDetectedMontageIfNeeded()가 이 몽타주를 사용하게 됩니다.
    return FighterFullMontage;
}