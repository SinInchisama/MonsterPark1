// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassReplicationTypes.h"
#include "MassClientBubbleHandler.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "MonsterReplicationTypes.generated.h"

USTRUCT()
struct MONSTERPARK_API FMonsterReplicatedAgent : public FReplicatedAgentBase
{
    GENERATED_BODY()

    UPROPERTY()
    FVector_NetQuantize Location = FVector::ZeroVector;

    UPROPERTY()
    uint8 Yaw = 0;

    UPROPERTY()
    float CurrentHealth = 0.f; // Status Fragment에서 가져올 체력
};

USTRUCT()
struct MONSTERPARK_API FMonsterFastArrayItem : public FMassFastArrayItemBase
{
    GENERATED_BODY()

    FMonsterFastArrayItem() = default;
    FMonsterFastArrayItem(const FMonsterReplicatedAgent& InAgent, const FMassReplicatedAgentHandle InHandle)
        : FMassFastArrayItemBase(InHandle), Agent(InAgent) {
    }

    typedef FMonsterReplicatedAgent FReplicatedAgentType;

    UPROPERTY()
    FMonsterReplicatedAgent Agent;
};

class MONSTERPARK_API FMonsterClientBubbleHandler : public TClientBubbleHandlerBase<FMonsterFastArrayItem>
{
public:
    typedef FMonsterFastArrayItem ItemType;

    void PostReplicatedChangeEntity(const FMassEntityView& EntityView, const FMonsterReplicatedAgent& ReplicatedAgent);
    virtual void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSectionsCount) override;
    virtual void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSectionsCount) override;
};

USTRUCT()
struct MONSTERPARK_API FMonsterClientBubbleSerializer : public FMassClientBubbleSerializerBase
{
    GENERATED_BODY()

    FMonsterClientBubbleSerializer()
    {
        BubbleHandler.Initialize(Items, *this);
    }

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FMonsterFastArrayItem, FMonsterClientBubbleSerializer>(Items, DeltaParms, *this);
    }

    UPROPERTY()
    TArray<FMonsterFastArrayItem> Items;

    FMonsterClientBubbleHandler BubbleHandler;
};

template<>
struct TStructOpsTypeTraits<FMonsterClientBubbleSerializer> : public TStructOpsTypeTraitsBase2<FMonsterClientBubbleSerializer>
{
    enum { WithNetDeltaSerializer = true };
};

