// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyBasicCharacter.h"
#include "PlaySubSystem.h"

#include "TimerManager.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "MyAnimInstance.h"

#include "MonsterAttributeSet.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterConditionFragment.h"
#include "Monster/Tag/FMonsterTag.h"

ACharacterBase::ACharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // ACharacter는 기본적으로 CapsuleComponent가 Root입니다.
    // 기존 SelectionBox가 충돌 판정용이었다면 캡슐의 크기를 조절합니다.
    GetCapsuleComponent()->InitCapsuleSize(50.f, 100.f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    // 내장 Mesh 컴포넌트 설정 (보통 캐릭터는 -90도 회전되어 있음)
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -100.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    // CharacterMovementComponent 설정 (Landscape 대응 핵심)
    GetCharacterMovement()->MaxWalkSpeed = 200.f;
    GetCharacterMovement()->bOrientRotationToMovement = true; // 이동 방향으로 자동 회전
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

        EnemyOutsideQuery = FMassEntityQuery(EntityManager.AsShared());
    }

    UWorld* World = GetWorld();
    if (World)
    {
        PlaySubsystem = World->GetSubsystem<UPlaySubSystem>();
    }

    EnemyQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EnemyQuery.AddRequirement<FMonsterStatusFragment>(EMassFragmentAccess::ReadWrite);
    EnemyQuery.AddRequirement<FMonsterConditionFragment>(EMassFragmentAccess::ReadWrite);
    EnemyQuery.AddTagRequirement<FMonsterTag>(EMassFragmentPresence::All);

    TargetQueryPtr = &EnemyQuery;
}

void ACharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (PlaySubsystem && LastGridKey != -1)
    {
        PlaySubsystem->RemoveHeroFromGrid(this, LastGridKey);
    }
    Super::EndPlay(EndPlayReason);
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

    if (!MoveDirection.IsNearlyZero() && Attacking)
    {
        // 핵심 변경: 직접 좌표를 계산하지 않고 입력값만 전달
        // CharacterMovementComponent가 중력과 지형(Landscape)을 계산하여 이동시킵니다.
        AddMovementInput(MoveDirection.GetSafeNormal(), 1.0f);

        if (PlaySubsystem)
        {
            PlaySubsystem->UpdateHeroLocation(this, LastGridKey, GetActorLocation());
        }

        UpdateAnimBPSpeed(1);
    }
    else
    {
        if (Attacking) FindEnemiesInArea();
        if (bEnemyDetected) Attack();
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
    /*UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
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

    TargetQueryPtr->ForEachEntityChunk(ExecContext, [this, MyLocation, RadiusSq, AttackPowerValue](FMassExecutionContext& Context)
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
        });*/

    if (!PlaySubsystem || !AbilitySystemComponent) return;

    UMassEntitySubsystem* MassSubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (!MassSubsystem) return;

    FMassEntityManager& EM = MassSubsystem->GetMutableEntityManager();
    FVector MyLoc = GetActorLocation();
    int64 MyKey = PlaySubsystem->GetGridKey(MyLoc);

    float Range = AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute());
    float RadiusSq = FMath::Square(Range);

    // 주변 9칸(자기 칸 포함)을 검사하여 범위 내 몬스터 수집
    TArray<FMassEntityHandle> CandidateMonsters;

    // 격자 좌표 복원 (X, Y)
    int32 CenterX = (int32)(MyKey >> 32);
    int32 CenterY = (int32)(MyKey & 0xFFFFFFFF);

    for (int32 x = -1; x <= 1; ++x)
    {
        for (int32 y = -1; y <= 1; ++y)
        {
            int64 CheckKey = ((int64)(CenterX + x) << 32) | (uint32)(CenterY + y);
            if (FGridData* Cell = PlaySubsystem->SpatialGrid.Find(CheckKey))
            {
                CandidateMonsters.Append(Cell->MonsterInCell);
            }
        }
    }

    // 찾아낸 몬스터 핸들들을 대상으로 데이터 접근
    for (FMassEntityHandle MonsterHandle : CandidateMonsters)
    {
        if (!EM.IsEntityValid(MonsterHandle)) continue;

        // 프래그먼트 데이터 직접 가져오기
        FTransformFragment* Transform = EM.GetFragmentDataPtr<FTransformFragment>(MonsterHandle);
        FMonsterConditionFragment* Condition = EM.GetFragmentDataPtr<FMonsterConditionFragment>(MonsterHandle);

        if (Transform && Condition)
        {
            float DistSq = FVector::DistSquared(MyLoc, Transform->GetTransform().GetLocation());
            if (DistSq <= RadiusSq)
            {
                // 공격 성공 로직
                Condition->Damage += DefaultAttackPower; // 공격력 적용
                bEnemyDetected = true;
                Attacking = false;
                break; // 한 마리만 찾으면 종료
            }
        }
    }
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

void ACharacterBase::MoveForward(float val)
{
	CurrentForwardInput = val;
}

void ACharacterBase::MoveRight(float val)
{
	CurrentRightInput = val;
}

void ACharacterBase::UpdateAnimBPSpeed(int val)
{
    USkeletalMeshComponent* CharacterMesh = FindComponentByClass<USkeletalMeshComponent>();
    if (!CharacterMesh)
    {
        return;
    }

    UAnimInstance* CurrentInst = CharacterMesh->GetAnimInstance();
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

void ACharacterBase::SetIsOutside(bool bOutside)
{
    bIsOutsideWall = bOutside;

 /*   if (bOutside) {
        TargetQueryPtr = &EnemyOutsideQuery;
    }
    else {
        TargetQueryPtr = &EnemyQuery;
    }*/
}
