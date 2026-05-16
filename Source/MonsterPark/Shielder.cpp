// Fill out your copyright notice in the Description page of Project Settings.


#include "Shielder.h"
#include "MonsterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MassCommonFragments.h"   

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

AShielder::AShielder()
{
    AuraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AuraComponent"));
    if (AuraComponent)
    {
        AuraComponent->SetupAttachment(GetRootComponent());
        AuraComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        AuraComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
        AuraComponent->bAutoActivate = false; 

        AuraComponent->SetRelativeLocation(FVector::ZeroVector);
    }
}

void AShielder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AShielder::BeginPlay()
{
    Super::BeginPlay();

    if (AuraComponent && AuraTemplate)
    {
        AuraComponent->SetAsset(AuraTemplate);
        AuraComponent->Activate(true);
    }
}


UAnimMontage* AShielder::GetDetectedMontage() const
{
    return ShielderFullMontage;
}

void AShielder::FindEnemiesInArea()
{
    Super::FindEnemiesInArea();

    UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (!EntitySubsystem || !AbilitySystemComponent) return;

    const float RangeValue = AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute());
    float RadiusSq = FMath::Square(500.0f);
    FVector MyLocation = GetActorLocation();

    FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
    FMassExecutionContext ExecContext(EntityManager, 0.0f);

    EnemyQuery.ForEachEntityChunk(ExecContext, [this, MyLocation, RadiusSq](FMassExecutionContext& Context)
        {
            const int32 NumEntities = Context.GetNumEntities();
            auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();
            auto Conditions = Context.GetMutableFragmentView<FMonsterStatusFragment>();

            for (int32 i = 0; i < NumEntities; ++i)
            {
                if (FVector::DistSquared(MyLocation, Transforms[i].GetTransform().GetLocation()) <= RadiusSq)
                {
                    Conditions[i].SpeedMultiplier = SlowMultiplier;
                }
                else
                {
                    Conditions[i].SpeedMultiplier = 400.0f;
                }
            }
        });
}

void AShielder::PlayDetectedMontageIfNeeded()
{
    if (!bEnemyDetected) return;

    if (USkeletalMeshComponent* CharacterMesh = FindComponentByClass<USkeletalMeshComponent>())
    {
        if (UAnimInstance* AnimInst = CharacterMesh->GetAnimInstance())
        {
            if (ShielderFullMontage && !AnimInst->Montage_IsPlaying(ShielderFullMontage))
            {
                AnimInst->Montage_Play(ShielderFullMontage);

                if (!bHasPlayedPassive)
                {
  
                    AnimInst->Montage_JumpToSection(FName("Passive"), ShielderFullMontage);
                    bHasPlayedPassive = true; 
                }
                else
                {
                    AnimInst->Montage_JumpToSection(FName("Attack"), ShielderFullMontage);
                }
            }
        }
    }
}