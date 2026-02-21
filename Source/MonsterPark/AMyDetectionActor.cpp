// Fill out your copyright notice in the Description page of Project Settings.


#include "AMyDetectionActor.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassEntityManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "MassExecutionContext.h"
#include "Components/BoxComponent.h"

#include "FMonsterConditionFragment.h"

// Sets default values
AAMyDetectionActor::AAMyDetectionActor()
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
}

// Called when the game starts or when spawned
void AAMyDetectionActor::BeginPlay()
{
	Super::BeginPlay();
	
    UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (EntitySubsystem)
    {
        FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
        // ������ �ʱ�ȭ�մϴ�. (SharedPtr ������ �Ŵ����� ����)
        // ����� ���ǵ� ������ �� TSharedPtr<FMassEntityManager>�� �޴� ������ ����ϰ� �˴ϴ�.
        EnemyQuery = FMassEntityQuery(EntityManager.AsShared());
    }

    // ���� �ʱ�ȭ�� �Ǿ����Ƿ� �䱸���� �߰��� �����մϴ�.
    EnemyQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EnemyQuery.AddRequirement<FMonsterConditionFragment>(EMassFragmentAccess::ReadWrite);
}

void AAMyDetectionActor::OnResumeAction()
{
    Attacking = true;
}

// Called every frame
void AAMyDetectionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    if(Attacking)
        FindEnemiesInArea();

}


void AAMyDetectionActor::FindEnemiesInArea()
{
    UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (!EntitySubsystem) return;

    //DetectedEnemies.Reset();

    FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();

    FMassExecutionContext ExecContext(EntityManager, 0.0f); 

    FVector MyLocation = GetActorLocation();
    float RadiusSq = FMath::Square(Range);

    EnemyQuery.ForEachEntityChunk(ExecContext, [this, MyLocation, RadiusSq](FMassExecutionContext& Context)
        {
            const int32 NumEntities = Context.GetNumEntities();
            TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
            //auto Transforms = Context.GetFragmentView<FTransformFragment>();

            for (int32 i = 0; i < NumEntities; ++i)
            {
                FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();

                if (FVector::DistSquared(MyLocation, EnemyLoc) <= RadiusSq)     // ���Ϳ� �Ÿ� ���
                {
                    bEnemyDetected = true;
                    UE_LOG(LogTemp, Warning, TEXT("Detected Count: %s"), *HeroDisplayName.ToString());
                    break;
                }
            }
        }); 

       // UE_LOG(LogTemp, Warning, TEXT("Detected Count: %d"), Count);

}

void AAMyDetectionActor::PlayDetectedMontageIfNeeded()
{
    // 적 감지 상태일 때 공격 몽타주 재생
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

void AAMyDetectionActor::SetMoveAnimClassIfNeeded()
{
    // 이동 시 유닛 타입에 맞는 AnimBP 적용
    if (USkeletalMeshComponent* SkeletalMesh = FindComponentByClass<USkeletalMeshComponent>())
    {
        TSubclassOf<UAnimInstance> MoveAnimClass = GetMoveAnimClass();
        if (MoveAnimClass && SkeletalMesh->GetAnimClass() != MoveAnimClass)
        {
            SkeletalMesh->SetAnimInstanceClass(MoveAnimClass);
        }
    }
}

UAnimMontage* AAMyDetectionActor::GetDetectedMontage() const
{
    // 유닛 타입에 맞는 몽타주 
    switch (UnitType)
    {
    case EDetectionUnitType::Knight:
        return KnightAnimMontage;
    case EDetectionUnitType::Thief:
        return ThiefAnimMontage;
    case EDetectionUnitType::BlackCat:
    default:
        return BlackCatAnimMontage;
    }
}

TSubclassOf<UAnimInstance> AAMyDetectionActor::GetMoveAnimClass() const
{
    // 유닛 타입에 맞는 AnimBP
    switch (UnitType)
    {
    case EDetectionUnitType::Knight:
        return KnightMoveAnimClass;
    case EDetectionUnitType::Thief:
        return ThiefMoveAnimClass;
    case EDetectionUnitType::BlackCat:
    default:
        return BlackCatMoveAnimClass;
    }
}

void AAMyDetectionActor::MoveForward(int val)
{
    FVector Forward = GetActorForwardVector();

    FVector NewLocation = GetActorLocation() + (Forward * (float)val *  GetWorld()->GetDeltaSeconds());

    SetActorLocation(NewLocation, true);
    // 이동 시 애니메이션
    SetMoveAnimClassIfNeeded();
    PlayDetectedMontageIfNeeded();
}

void AAMyDetectionActor::MoveRight(int val)
{
    FVector RightVector = GetActorRightVector();

    FVector NewLocation = GetActorLocation() + (RightVector * (float)val  * GetWorld()->GetDeltaSeconds());

    SetActorLocation(NewLocation, true);
    // 이동 시 애니메이션
    SetMoveAnimClassIfNeeded();
    PlayDetectedMontageIfNeeded();
}
