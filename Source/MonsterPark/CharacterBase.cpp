// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "MonsterAttributeSet.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "MyAnimInstance.h"
#include "MonsterPark/Monster/Fragment/FMonsterConditionFragment.h"
#include "MyBasicCharacter.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    USceneComponent* DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DummyRoot"));
    RootComponent = DummyRoot;

    SelectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SelectionBox"));
    SelectionBox->SetupAttachment(RootComponent);

    SelectionBox->SetBoxExtent(FVector(50.0f, 50.0f, 100.0f));

    SelectionBox->SetCollisionProfileName(TEXT("Custom"));
    SelectionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SelectionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    SelectionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    // Add the ability system component
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(AscReplicationMode);

    AttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("AttributeSet"));
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
    Super::BeginPlay();

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
        AbilitySystemComponent->SetNumericAttributeBase(UMonsterAttributeSet::GetAttackPowerAttribute(), DefaultAttackPower);
        AbilitySystemComponent->SetNumericAttributeBase(UMonsterAttributeSet::GetAttackSpeedAttribute(), DefaultAttackSpeed);
        AbilitySystemComponent->SetNumericAttributeBase(UMonsterAttributeSet::GetRangeAttribute(), DefaultRange);
        AbilitySystemComponent->SetNumericAttributeBase(UMonsterAttributeSet::GetCostAttribute(), DefaultCost);
    }

    UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (EntitySubsystem)
    {
        FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
        EnemyQuery = FMassEntityQuery(EntityManager.AsShared());
    }

    EnemyQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EnemyQuery.AddRequirement<FMonsterConditionFragment>(EMassFragmentAccess::ReadWrite);
}

void ACharacterBase::OnResumeAction()
{
    Attacking = true;
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector MoveDirection = FVector(CurrentForwardInput, CurrentRightInput, 0.0f);

    if (!MoveDirection.IsNearlyZero() && SelectionBox && Attacking)
    {
        MoveDirection = MoveDirection.GetSafeNormal();

        FRotator TargetRotation = MoveDirection.Rotation();
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 10.0f));

        FVector BoxStartWorldLoc = SelectionBox->GetComponentLocation();

        FVector Delta = MoveDirection * 200.0f * DeltaTime;
        FHitResult Hit;
        SelectionBox->MoveComponent(Delta, GetActorRotation(), true, &Hit);

        FVector BoxEndWorldLoc = SelectionBox->GetComponentLocation();
        FVector ActualMoveDelta = BoxEndWorldLoc - BoxStartWorldLoc;

        if (!ActualMoveDelta.IsNearlyZero())
        {
            AddActorWorldOffset(ActualMoveDelta, false);
            SelectionBox->SetRelativeLocation(FVector::ZeroVector);
        }

        UpdateAnimBPSpeed(1);
        SetMoveAnimClassIfNeeded();
    }
    else
    {
        if (Attacking)
        {
            FindEnemiesInArea();
        }

        if (bEnemyDetected)
        {
            Attack();
        }

        UpdateAnimBPSpeed(0);
    }

    CurrentForwardInput = 0.0f;
    CurrentRightInput = 0.0f;
}

void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ACharacterBase::Attack_Melee);
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ACharacterBase::Attack_Melee()
{
    UE_LOG(LogTemp, Warning, TEXT("Attack_Melee Called!"));
    GetWorldTimerManager().SetTimer(TH_Attack_End, this, &ACharacterBase::Attack_End, 1.0f, false);
}

void ACharacterBase::Attack_End()
{
    UE_LOG(LogTemp, Warning, TEXT("Attack_End Called!"));
}

void ACharacterBase::FindEnemiesInArea()
{
    UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (!EntitySubsystem)
    {
        return;
    }

    const float RangeValue = AbilitySystemComponent
        ? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute())
        : DefaultRange;

    const float AttackPowerValue = AbilitySystemComponent
        ? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetAttackPowerAttribute())
        : DefaultAttackPower;

    FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
    FMassExecutionContext ExecContext(EntityManager, 0.0f);

    FVector MyLocation = GetActorLocation();
    float RadiusSq = FMath::Square(RangeValue);

    EnemyQuery.ForEachEntityChunk(ExecContext, [this, MyLocation, RadiusSq, AttackPowerValue](FMassExecutionContext& Context)
        {
            const int32 NumEntities = Context.GetNumEntities();
            TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
            TArrayView<FMonsterConditionFragment> Condtions = Context.GetMutableFragmentView<FMonsterConditionFragment>();

            for (int32 i = 0; i < NumEntities; ++i)
            {
                FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();

                if (FVector::DistSquared(MyLocation, EnemyLoc) <= RadiusSq)
                {
                    bEnemyDetected = true;
                    Condtions[i].Damage += AttackPowerValue;
                    Attacking = false;
                    break;
                }
            }
        });
}

void ACharacterBase::PlayDetectedMontageIfNeeded()
{
    if (!bEnemyDetected)
    {
        return;
    }

    if (USkeletalMeshComponent* SkeletalMesh = FindComponentByClass<USkeletalMeshComponent>())
    {
        if (UAnimInstance* AnimInstance = SkeletalMesh->GetAnimInstance())
        {
            UAnimMontage* TargetMontage = GetDetectedMontage();
            if (TargetMontage && !AnimInstance->Montage_IsPlaying(TargetMontage))
            {
                AnimInstance->Montage_Play(TargetMontage);
            }
        }
    }
}

void ACharacterBase::SetMoveAnimClassIfNeeded()
{
    if (USkeletalMeshComponent* SkeletalMesh = FindComponentByClass<USkeletalMeshComponent>())
    {
        TSubclassOf<UAnimInstance> MoveAnimClass = GetMoveAnimClass();
        if (MoveAnimClass && SkeletalMesh->GetAnimClass() != MoveAnimClass)
        {
            SkeletalMesh->SetAnimInstanceClass(MoveAnimClass);
        }
    }
}

void ACharacterBase::Attack()
{
    PlayDetectedMontageIfNeeded();
    bEnemyDetected = false;

    float AttackSpeedValue = AbilitySystemComponent
        ? AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetAttackSpeedAttribute())
        : DefaultAttackSpeed;

    float AttackInterval = 1.0f / FMath::Max(AttackSpeedValue, KINDA_SMALL_NUMBER);

    GetWorldTimerManager().SetTimer(AttackDelayTimerHandle, this, &ACharacterBase::ResetAttack, AttackInterval, false);
}

void ACharacterBase::ResetAttack()
{
    Attacking = true;
}

UAnimMontage* ACharacterBase::GetDetectedMontage() const
{
    return AnimMontage;
}

TSubclassOf<UAnimInstance> ACharacterBase::GetMoveAnimClass() const
{
    return AnimClass;
}

void ACharacterBase::MoveForward(int val)
{
    CurrentForwardInput = (float)val;
}

void ACharacterBase::MoveRight(int val)
{
    CurrentRightInput = (float)val;
}

void ACharacterBase::UpdateAnimBPSpeed(int val)
{
    USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh)
    {
        return;
    }

    UAnimInstance* CurrentInst = Mesh->GetAnimInstance();
    if (!CurrentInst)
    {
        return;
    }

    if (CurrentInst->IsA(GetMoveAnimClass()))
    {
        if (UMyAnimInstance* MyInst = Cast<UMyAnimInstance>(CurrentInst))
        {
            MyInst->speed = (val != 0) ? 200.0f : 0.0f;
        }
    }
}