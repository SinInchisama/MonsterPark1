// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterClientBubbleInfo.h"
#include "Net/UnrealNetwork.h"

AMonsterClientBubbleInfo::AMonsterClientBubbleInfo(const FObjectInitializer& ObjectInitializer)
{
    Serializers.Add(&BubbleSerializer);
}

void AMonsterClientBubbleInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    FDoRepLifetimeParams SharedParams;
    SharedParams.bIsPushBased = true;

    DOREPLIFETIME_WITH_PARAMS_FAST(AMonsterClientBubbleInfo, BubbleSerializer, SharedParams);
}