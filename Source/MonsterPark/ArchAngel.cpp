// Fill out your copyright notice in the Description page of Project Settings.


#include "ArchAngel.h"
#include "ArchAngelSkillAbility.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.h"
#include "MyBasicCharacter.h"
#include "Animation/AnimInstance.h"

 AArchAngel::AArchAngel()
{
	SkillAbilityClass = UArchAngelSkillAbility::StaticClass();
}

void AArchAngel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AArchAngel::FindEnemiesInArea()
{
	Super::FindEnemiesInArea();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AMyBasicCharacter* MyChar = PC ? Cast<AMyBasicCharacter>(PC->GetPawn()) : nullptr;
	if (!MyChar)
	{
		return;
	}

	const FVector MyLocation = GetActorLocation();
	const float BuffRadiusSq = FMath::Square(BuffRadius);
	for (ACharacterBase* Hero : MyChar->MySummonedHero)
	{
		if (!Hero)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = Hero->GetAbilitySystemComponent();
		if (!TargetASC)
		{
			continue;
		}

		const float BaseAttackSpeed = Hero->DefaultAttackSpeed;
		const bool bInRange = FVector::DistSquared(MyLocation, Hero->GetActorLocation()) <= BuffRadiusSq;
		const float NewAttackSpeed = bInRange ? BaseAttackSpeed * AttackSpeedBuffMultiplier : BaseAttackSpeed;
		TargetASC->SetNumericAttributeBase(UMonsterAttributeSet::GetAttackSpeedAttribute(), NewAttackSpeed);
	}
}

void AArchAngel::UseSkill()
{
	if (!AbilitySystemComponent || !SkillAbilityClass)
	{
		return;
	}

	FGameplayAbilitySpec* SkillSpec = AbilitySystemComponent->FindAbilitySpecFromClass(SkillAbilityClass);
	UArchAngelSkillAbility* SkillAbility = SkillSpec
		? Cast<UArchAngelSkillAbility>(SkillSpec->GetPrimaryInstance())
		: nullptr;
	if (!SkillAbility || !SkillAbility->IsCooldownReady(GetWorld()))
	{
		return;
	}

	bSkillRequested = true;

	const bool bActivated = AbilitySystemComponent->TryActivateAbilityByClass(SkillAbilityClass);
	if (!bActivated)
	{
		bSkillRequested = false;
		return;
	}

	if (SkillMontage)
	{
		if (USkeletalMeshComponent* CharacterMesh = GetMesh())
		{
			if (UAnimInstance* AnimInst = CharacterMesh->GetAnimInstance())
			{
				AnimInst->Montage_Play(SkillMontage);
				AnimInst->Montage_JumpToSection(FName("Skill"), SkillMontage);
			}
		}
	}
}

bool AArchAngel::ExecuteSkill()
{
	if (!MinionClass)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.0f;
	const FRotator SpawnRotation = GetActorRotation();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	ACharacterBase* Minion = World->SpawnActor<ACharacterBase>(MinionClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!Minion)
	{
		return false;
	}

	Minion->SetLifeSpan(MinionLifetime);

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (AMyBasicCharacter* MyChar = Cast<AMyBasicCharacter>(PC->GetPawn()))
		{
			MyChar->SetSummonedActor(Minion);
		}
	}

	return true;
}

bool AArchAngel::IsSkillRequested() const
{
	return bSkillRequested;
}

void AArchAngel::ClearSkillRequest()
{
	bSkillRequested = false;
}

bool AArchAngel::HasMinionClass() const
{
	return MinionClass != nullptr;
}

UAnimMontage* AArchAngel::GetDetectedMontage() const
{
	return ArchAngelFullMontage;
}

void AArchAngel::PlayDetectedMontageIfNeeded()
{
	PlayDetectedMontageSection(ArchAngelFullMontage, bHasPlayedPassive);
}

