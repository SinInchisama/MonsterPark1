// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassClientBubbleInfoBase.h"
#include "MonsterPark/Monster/Replicated/MonsterReplicationTypes.h"
#include "MonsterClientBubbleInfo.generated.h"

UCLASS()
class MONSTERPARK_API AMonsterClientBubbleInfo : public AMassClientBubbleInfoBase
{
    GENERATED_BODY()

public:
    AMonsterClientBubbleInfo(const FObjectInitializer& ObjectInitializer);

    FMonsterClientBubbleSerializer& GetBubbleSerializer() { return BubbleSerializer; }

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


    UPROPERTY(Replicated, Transient)
    FMonsterClientBubbleSerializer BubbleSerializer;
};