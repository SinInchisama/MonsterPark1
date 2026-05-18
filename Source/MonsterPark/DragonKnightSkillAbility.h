// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DragonKnightSkillAbility.generated.h"

UCLASS()
class MONSTERPARK_API UDragonKnightSkillAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UDragonKnightSkillAbility();

	bool IsCooldownReady(const UWorld* World) const;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		OUT FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float CooldownDuration = 20.0f;

private:
	mutable float LastActivationTime = -FLT_MAX;
};
