// Fill out your copyright notice in the Description page of Project Settings.

#include "DragonKnightSkillAbility.h"
#include "DragonKnight.h"

UDragonKnightSkillAbility::UDragonKnightSkillAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UDragonKnightSkillAbility::IsCooldownReady(const UWorld* World) const
{
	if (!World)
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	return (CurrentTime - LastActivationTime) >= CooldownDuration;
}

bool UDragonKnightSkillAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const ADragonKnight* DragonKnight = ActorInfo && ActorInfo->AvatarActor.IsValid()
		? Cast<ADragonKnight>(ActorInfo->AvatarActor.Get())
		: nullptr;
	if (!DragonKnight || !DragonKnight->IsSkillRequested())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return IsCooldownReady(World);
}

void UDragonKnightSkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ADragonKnight* DragonKnight = Cast<ADragonKnight>(ActorInfo->AvatarActor.Get());
	if (!DragonKnight)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	DragonKnight->ExecuteSkill();
	LastActivationTime = World->GetTimeSeconds();
	DragonKnight->ClearSkillRequest();

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
