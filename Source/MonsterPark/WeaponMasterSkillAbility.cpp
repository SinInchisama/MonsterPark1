// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponMasterSkillAbility.h"
#include "WeaponMaster.h"

UWeaponMasterSkillAbility::UWeaponMasterSkillAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UWeaponMasterSkillAbility::IsCooldownReady(const UWorld* World) const
{
	if (!World)
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	return (CurrentTime - LastActivationTime) >= CooldownDuration;
}

bool UWeaponMasterSkillAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AWeaponMaster* WeaponMaster = ActorInfo && ActorInfo->AvatarActor.IsValid()
		? Cast<AWeaponMaster>(ActorInfo->AvatarActor.Get())
		: nullptr;
	if (!WeaponMaster || !WeaponMaster->IsSkillRequested())
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

void UWeaponMasterSkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AWeaponMaster* WeaponMaster = Cast<AWeaponMaster>(ActorInfo->AvatarActor.Get());
	if (!WeaponMaster)
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

	if (WeaponMaster->ExecuteSkill())
	{
		LastActivationTime = World->GetTimeSeconds();
	}
	WeaponMaster->ClearSkillRequest();

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
