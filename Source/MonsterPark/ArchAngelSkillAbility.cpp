// Fill out your copyright notice in the Description page of Project Settings.

#include "ArchAngelSkillAbility.h"
#include "ArchAngel.h"

UArchAngelSkillAbility::UArchAngelSkillAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UArchAngelSkillAbility::IsCooldownReady(const UWorld* World) const
{
	if (!World)
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	return (CurrentTime - LastActivationTime) >= CooldownDuration;
}

bool UArchAngelSkillAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AArchAngel* ArchAngel = ActorInfo && ActorInfo->AvatarActor.IsValid()
		? Cast<AArchAngel>(ActorInfo->AvatarActor.Get())
		: nullptr;
	if (!ArchAngel || !ArchAngel->IsSkillRequested() || !ArchAngel->HasMinionClass())
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

void UArchAngelSkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AArchAngel* ArchAngel = Cast<AArchAngel>(ActorInfo->AvatarActor.Get());
	if (!ArchAngel)
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

	ArchAngel->ExecuteSkill();
	LastActivationTime = World->GetTimeSeconds();

	ArchAngel->ClearSkillRequest();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
