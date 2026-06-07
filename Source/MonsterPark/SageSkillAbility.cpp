// Fill out your copyright notice in the Description page of Project Settings.

#include "SageSkillAbility.h"
#include "Sage.h"

USageSkillAbility::USageSkillAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool USageSkillAbility::IsCooldownReady(const UWorld* World) const
{
	if (!World)
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	return (CurrentTime - LastActivationTime) >= CooldownDuration;
}

bool USageSkillAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const ASage* Sage = ActorInfo && ActorInfo->AvatarActor.IsValid()
		? Cast<ASage>(ActorInfo->AvatarActor.Get())
		: nullptr;
	if (!Sage || !Sage->IsSkillRequested())
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

void USageSkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ASage* Sage = Cast<ASage>(ActorInfo->AvatarActor.Get());
	if (!Sage)
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

	if (Sage->ExecuteSkill())
	{
		LastActivationTime = World->GetTimeSeconds();
	}
	Sage->ClearSkillRequest();

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
