// Fill out your copyright notice in the Description page of Project Settings.

#include "DragonKnight.h"
#include "DragonKnightSkillAbility.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassEntityManager.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MassCommonFragments.h"
#include "Animation/AnimInstance.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/ConstructorHelpers.h"

ADragonKnight::ADragonKnight()
{
	SkillAbilityClass = UDragonKnightSkillAbility::StaticClass();

	static ConstructorHelpers::FClassFinder<AActor> DragonBreathVFXBP(TEXT("/Game/VFX/DragonBreath/BP_DragonBreathVFX"));
	if (DragonBreathVFXBP.Succeeded())
	{
		DragonBreathVFXClass = DragonBreathVFXBP.Class;
	}
}

void ADragonKnight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADragonKnight::FindEnemiesInArea()
{
	if (bSkillRequested)
	{
		return;
	}

	if (SkillMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		if (GetMesh()->GetAnimInstance()->Montage_IsPlaying(SkillMontage))
		{
			return;
		}
	}

	Super::FindEnemiesInArea();
}

void ADragonKnight::UseSkill()
{
	if (!SkillAbilityClass)
	{
		SkillAbilityClass = UDragonKnightSkillAbility::StaticClass();
	}

	if (!AbilitySystemComponent || !SkillAbilityClass)
	{
		return;
	}

	FGameplayAbilitySpec* SkillSpec = AbilitySystemComponent->FindAbilitySpecFromClass(SkillAbilityClass);
	if (!SkillSpec && HasAuthority())
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(SkillAbilityClass, 1, 1, this));
		SkillSpec = AbilitySystemComponent->FindAbilitySpecFromClass(SkillAbilityClass);
	}
	UDragonKnightSkillAbility* SkillAbility = SkillSpec
		? Cast<UDragonKnightSkillAbility>(SkillSpec->GetPrimaryInstance())
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

				float DelayTime = 3.f;//SkillMontage->GetTimeAtFrame(35);

				ExecuteSkill();
			}
		}
	}
}

bool ADragonKnight::ExecuteSkill()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	SpawnDragonBreathVFX();
	StartBreathDamage();

	return true;
}

bool ADragonKnight::IsSkillRequested() const
{
	return bSkillRequested;
}

void ADragonKnight::ClearSkillRequest()
{
	bSkillRequested = false;
}

UAnimMontage* ADragonKnight::GetDetectedMontage() const
{
	return DragonKnightFullMontage;
}

void ADragonKnight::PlayDetectedMontageIfNeeded()
{
	if (bSkillRequested)
	{
		return;
	}

	if (SkillMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		if (GetMesh()->GetAnimInstance()->Montage_IsPlaying(SkillMontage))
		{
			return;
		}
	}

	PlayDetectedMontageSection(DragonKnightFullMontage, bHasPlayedPassive);
}

void ADragonKnight::SpawnDragonBreathVFX()
{
	UWorld* World = GetWorld();
	if (!World || (!DragonBreathVFXClass && !BasicAttackFireStormTemplate))
	{
		return;
	}

	FVector SpawnLocation = GetActorLocation()
		+ GetActorForwardVector() * DragonBreathFallbackOffset.X
		+ GetActorRightVector() * DragonBreathFallbackOffset.Y
		+ FVector(0.0f, 0.0f, DragonBreathFallbackOffset.Z);
	FRotator SpawnRotation = GetActorRotation();
	bool bAttachToMouth = false;

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (CharacterMesh && (CharacterMesh->DoesSocketExist(DragonBreathSocketName) || CharacterMesh->GetBoneIndex(DragonBreathSocketName) != INDEX_NONE))
	{
		SpawnLocation = CharacterMesh->GetSocketLocation(DragonBreathSocketName);
		bAttachToMouth = true;
	}

	if (BasicAttackFireStormTemplate)
	{
		const FVector FireStormLocation = GetActorLocation()
			+ GetActorForwardVector() * BasicAttackFireStormOffset.X
			+ GetActorRightVector() * BasicAttackFireStormOffset.Y
			+ FVector(0.0f, 0.0f, BasicAttackFireStormOffset.Z);

		UParticleSystemComponent* FireStorm = UGameplayStatics::SpawnEmitterAtLocation(
			World,
			BasicAttackFireStormTemplate,
			FireStormLocation,
			FRotator::ZeroRotator,
			BasicAttackFireStormScale,
			true
		);

		if (FireStorm)
		{
			FireStorm->SetVisibility(true, true);
			FireStorm->SetComponentTickEnabled(true);
			FireStorm->ActivateSystem(true);

			if (BasicAttackFireStormDuration > 0.0f)
			{
				TWeakObjectPtr<UParticleSystemComponent> FireStormPtr(FireStorm);
				FTimerHandle FireStormTimerHandle;
				GetWorldTimerManager().SetTimer(
					FireStormTimerHandle,
					[FireStormPtr]()
					{
						if (FireStormPtr.IsValid())
						{
							FireStormPtr->DeactivateSystem();
						}
					},
					BasicAttackFireStormDuration,
					false
				);
			}
		}
	}

	if (!DragonBreathVFXClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* DragonBreathVFX = World->SpawnActor<AActor>(DragonBreathVFXClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!DragonBreathVFX)
	{
		return;
	}

	DragonBreathVFX->SetActorHiddenInGame(false);
	DragonBreathVFX->SetActorTickEnabled(true);
	DragonBreathVFX->SetActorEnableCollision(false);

	if (bAttachToMouth && CharacterMesh)
	{
		DragonBreathVFX->AttachToComponent(CharacterMesh, FAttachmentTransformRules::KeepWorldTransform, DragonBreathSocketName);
	}

	TArray<UNiagaraComponent*> NiagaraComponents;
	DragonBreathVFX->GetComponents<UNiagaraComponent>(NiagaraComponents);
	for (UNiagaraComponent* NiagaraComponent : NiagaraComponents)
	{
		if (NiagaraComponent)
		{
			NiagaraComponent->SetVisibility(true, true);
			NiagaraComponent->SetRenderingEnabled(true);
			NiagaraComponent->SetComponentTickEnabled(true);
			NiagaraComponent->ResetSystem();
			NiagaraComponent->Activate(true);
		}
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	DragonBreathVFX->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent)
		{
			PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			PrimitiveComponent->SetGenerateOverlapEvents(false);
			PrimitiveComponent->SetSimulatePhysics(false);

			if (PrimitiveComponent->GetFName() == FName("DamageVolume"))
			{
				PrimitiveComponent->SetVisibility(false, true);
				PrimitiveComponent->SetHiddenInGame(true, true);
				PrimitiveComponent->Deactivate();
				continue;
			}

			PrimitiveComponent->SetVisibility(true, true);
			PrimitiveComponent->SetHiddenInGame(false, true);
			PrimitiveComponent->SetComponentTickEnabled(true);
			PrimitiveComponent->Activate(true);
		}
	}

	if (DragonBreathVFXLifeTime > 0.0f)
	{
		DragonBreathVFX->SetLifeSpan(DragonBreathVFXLifeTime);
	}
}

void ADragonKnight::StartBreathDamage()
{
	if (BreathTickInterval <= 0.0f)
	{
		return;
	}

	TotalBreathTicks = FMath::Max(1, FMath::CeilToInt(BreathDuration / BreathTickInterval));
	RemainingBreathTicks = TotalBreathTicks;

	ApplyBreathDamageTick(); 
	GetWorldTimerManager().SetTimer(BreathTimerHandle, this, &ADragonKnight::ApplyBreathDamageTick, BreathTickInterval, true);
}

void ADragonKnight::ApplyBreathDamageTick()
{
	if (RemainingBreathTicks <= 0)
	{
		GetWorldTimerManager().ClearTimer(BreathTimerHandle);
		ClearSkillRequest();
		return;
	}

	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		--RemainingBreathTicks;
		return;
	}

	const float RangeValue = 1000.0f;
	const float RangeSq = FMath::Square(RangeValue);

	const float BreathConeAngle = 90.0f;
	const float HalfAngleRadians = FMath::DegreesToRadians(BreathConeAngle * 0.5f);
	const float MinDotProduct = FMath::Cos(HalfAngleRadians);

	float ElapsedTime = (TotalBreathTicks - RemainingBreathTicks) * BreathTickInterval;

	float YawOffset = (ElapsedTime < 2.5f) ? 45.0f : -45.0f;

	FVector BaseForward = GetActorForwardVector();
	FRotator RotationOffset(0.0f, YawOffset, 0.0f);
	FVector ForwardDir = RotationOffset.RotateVector(BaseForward).GetSafeNormal2D();

	FVector Origin = GetActorLocation();

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	FMassExecutionContext ExecContext(EntityManager, 0.0f);

	TargetQueryPtr->ForEachEntityChunk(ExecContext, [this, Origin, ForwardDir, RangeSq, MinDotProduct](FMassExecutionContext& Context)
		{
			const int32 NumEntities = Context.GetNumEntities();
			TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
			TArrayView<FMonsterStatusFragment> Statuses = Context.GetMutableFragmentView<FMonsterStatusFragment>();

			for (int32 i = 0; i < NumEntities; ++i)
			{
				FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();

				if (FVector::DistSquared(Origin, EnemyLoc) <= RangeSq)
				{
					FVector DirToEnemy = (EnemyLoc - Origin).GetSafeNormal2D();
					if (FVector::DotProduct(ForwardDir, DirToEnemy) >= MinDotProduct)
					{
						Statuses[i].PendingAoEDamage += SkillDamage;
						Statuses[i].PendingBleedDamage += Statuses[i].MaxHealt * BurnPercent;
					}
				}
			}
		});

	--RemainingBreathTicks;
}