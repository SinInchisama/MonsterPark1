// Fill out your copyright notice in the Description page of Project Settings.


#include "Berserker.h"

void ABerserker::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
};

UAnimMontage* ABerserker::GetDetectedMontage() const
{
    return BerserkerFullMontage;
}