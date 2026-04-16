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

#include "MonsterPark/Monster/Fragment/FMonsterConditionFragment.h"

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

    FVector MoveDirection = FVector(CurrentForwardInput, CurrentRightInput, 0.0f);
    
    if (!MoveDirection.IsNearlyZero() && SelectionBox&&Attacking)
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
    else {
        if (Attacking)
            FindEnemiesInArea();
        if (bEnemyDetected) {
            Attack();
        }
        UpdateAnimBPSpeed(0);
    }
    CurrentForwardInput = 0.0f;
    CurrentRightInput = 0.0f;
    //bEnemyDetected = false;
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
            TArrayView<FMonsterConditionFragment> Condtions = Context.GetMutableFragmentView<FMonsterConditionFragment>();
            //auto Transforms = Context.GetFragmentView<FTransformFragment>();

            for (int32 i = 0; i < NumEntities; ++i)
            {
                FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();

                if (FVector::DistSquared(MyLocation, EnemyLoc) <= RadiusSq)     // ���Ϳ� �Ÿ� ���
                {
                    bEnemyDetected = true;
                    Condtions[i].Damage += Damage;
                    Attacking = false;
                    //UE_LOG(LogTemp, Warning, TEXT("Detected Count: %s"), *HeroDisplayName.ToString());
                    break;
                }
            }
        }); 
}

void AAMyDetectionActor::PlayDetectedMontageIfNeeded()
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

void AAMyDetectionActor::SetMoveAnimClassIfNeeded()
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

void AAMyDetectionActor::Attack()
{
    PlayDetectedMontageIfNeeded();
    bEnemyDetected =false;

    float AttackInterval = 1.0f / AttackSpeed;

    GetWorldTimerManager().SetTimer(AttackDelayTimerHandle, this, &AAMyDetectionActor::ResetAttack, AttackInterval, false);
}

void AAMyDetectionActor::ResetAttack()
{
    Attacking = true;
}

UAnimMontage* AAMyDetectionActor::GetDetectedMontage() const
{
    return nullptr;
}

TSubclassOf<UAnimInstance> AAMyDetectionActor::GetMoveAnimClass() const
{
    return nullptr;
}

void AAMyDetectionActor::MoveForward(int val)
{
    CurrentForwardInput = (float)val;
}

void AAMyDetectionActor::MoveRight(int val)
{
    CurrentRightInput = (float)val;
}



void AAMyDetectionActor::UpdateAnimBPSpeed(int val)
{
    USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh) return;

    UAnimInstance* CurrentInst = Mesh->GetAnimInstance();
    if (!CurrentInst) return;

    if (CurrentInst->IsA(GetMoveAnimClass()))
    {
        if (UMyAnimInstance* MyInst = Cast<UMyAnimInstance>(CurrentInst))
        {
            MyInst->speed = (val != 0) ? 200.0f : 0.0f;
        }
    }
}