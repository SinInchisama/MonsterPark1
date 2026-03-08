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

    FVector MoveDirection = FVector(CurrentForwardInput, CurrentRightInput, 0.0f);
    
    if (!MoveDirection.IsNearlyZero() && SelectionBox&&Attacking)
    {
        MoveDirection = MoveDirection.GetSafeNormal();

        // 1. 액터(부모) 회전: 입력 방향으로 부드럽게 회전
        FRotator TargetRotation = MoveDirection.Rotation();
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 10.0f));

        // 2. 이동 전, 현재 박스의 '월드 위치'를 기억해둡니다.
        FVector BoxStartWorldLoc = SelectionBox->GetComponentLocation();

        // 3. 자식 박스 이동 (회전값도 현재 액터 회전값을 넣어 박스도 같이 회전시킴)
        FVector Delta = MoveDirection * 200.0f * DeltaTime;
        FHitResult Hit;
        // 박스도 액터와 같은 회전값을 가지도록 GetActorRotation()을 전달
        SelectionBox->MoveComponent(Delta, GetActorRotation(), true, &Hit);

        // 4. [핵심] 이동 후 박스의 '월드 위치'가 얼마나 변했는지 계산합니다.
        FVector BoxEndWorldLoc = SelectionBox->GetComponentLocation();
        FVector ActualMoveDelta = BoxEndWorldLoc - BoxStartWorldLoc;

        if (!ActualMoveDelta.IsNearlyZero())
        {
            // 박스가 실제로 움직인 '월드 거리'만큼 액터 본체를 이동시킵니다.
            // 상대 좌표(Relative)를 쓰지 않기 때문에 X축으로 쏠리는 버그가 사라집니다.
            AddActorWorldOffset(ActualMoveDelta, false);

            // 5. 액터를 옮겼으니 박스는 다시 부모(DummyRoot) 위치인 0,0,0으로 리셋
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

//void AAMyDetectionActor::MoveForward(int val)
//{
//    if (val == 0 || !SelectionBox) return;
//
//    FVector Forward = GetActorForwardVector();
//    FVector Delta = Forward * (float)val * 200.0f * GetWorld()->GetDeltaSeconds();
//
//    FHitResult Hit;
//    // 1. 박스를 먼저 이동시킵니다. (이 시점에서 박스만 Delta만큼 이동함)
//    bool bBlocked = SelectionBox->MoveComponent(Delta, GetActorRotation(), true, &Hit);
//
//    if (bBlocked && Hit.bBlockingHit)
//    {
//        // 벽에 부딪혔다면 액터 이동을 취소하고 종료
//        UE_LOG(LogTemp, Warning, TEXT("Blocked by: %s"), *Hit.GetActor()->GetName());
//        return;
//    }
//
//    // 2. [핵심] 박스가 이동한 후의 '상대 위치'를 다시 0으로 리셋하면서 
//    // 그만큼 액터(부모) 전체를 옮겨줍니다.
//    FVector BoxRelativeLoc = SelectionBox->GetRelativeLocation();
//    if (!BoxRelativeLoc.IsNearlyZero())
//    {
//        // 박스가 이동한 만큼 액터를 이동시키고
//        AddActorWorldOffset(BoxRelativeLoc);
//        // 박스는 다시 부모(DummyRoot) 위치인 0,0,0으로 되돌립니다.
//        SelectionBox->SetRelativeLocation(FVector::ZeroVector);
//    }
//
//    UpdateAnimBPSpeed(1);
//    SetMoveAnimClassIfNeeded();
//}
//
//void AAMyDetectionActor::MoveRight(int val)
//{
//    // 1. 유효성 검사
//    if (val == 0 || !SelectionBox) return;
//
//    // 2. 방향 및 이동량 계산
//    FVector RightVector = GetActorRightVector();
//    FVector Delta = RightVector * (float)val * 200.0f * GetWorld()->GetDeltaSeconds();
//
//    FHitResult Hit;
//
//    // 3. SelectionBox만 물리 체크하며 이동 시도
//    // 이 시점에서 SelectionBox의 RelativeLocation(상대 위치)이 변하게 됩니다.
//    bool bBlocked = SelectionBox->MoveComponent(Delta, GetActorRotation(), true, &Hit);
//
//    // 4. 충돌 판정
//    if (bBlocked && Hit.bBlockingHit)
//    {
//
//        // 부딪혔을 때는 자식(박스)이 이동한 거리를 무시하고 제자리로 되돌립니다.
//        SelectionBox->SetRelativeLocation(FVector::ZeroVector);
//        return;
//    }
//
//    // 5. 이동 거리 동기화 (핵심)
//    // 박스가 이동한 거리(RelativeLocation)를 가져옵니다.
//    FVector BoxRelativeLoc = SelectionBox->GetRelativeLocation();
//
//    // 박스가 이동한 만큼 액터 전체를 월드 좌표에서 이동시킵니다.
//    if (!BoxRelativeLoc.IsNearlyZero())
//    {
//        AddActorWorldOffset(BoxRelativeLoc);
//
//        // [중요] 액터를 옮겼으므로 자식인 박스는 다시 원점(0,0,0)으로 리셋합니다.
//        // 이렇게 해야 다음 프레임에서 박스가 또 앞서나가는 현상이 생기지 않습니다.
//        SelectionBox->SetRelativeLocation(FVector::ZeroVector);
//    }
//
//    UpdateAnimBPSpeed(1);
//    SetMoveAnimClassIfNeeded();
//}


void AAMyDetectionActor::UpdateAnimBPSpeed(int val)
{
    // 1. 메쉬 컴포넌트 가져오기
    USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh) return;

    // 2. 현재 메쉬가 돌리고 있는 애니메이션 인스턴스 가져오기
    UAnimInstance* CurrentInst = Mesh->GetAnimInstance();
    if (!CurrentInst) return;

    // 3. 현재 인스턴스가 우리가 지정한 'BlackCatMoveAnimClass'의 자식인지 확인
    // 이렇게 하면 직접적인 클래스 이름(UMyAnimInstance)을 하드코딩하지 않아도 됩니다.
    if (CurrentInst->IsA(GetMoveAnimClass()))
    {
        // 부모 타입인 UMyAnimInstance로 형변환하여 speed에 접근
        if (UMyAnimInstance* MyInst = Cast<UMyAnimInstance>(CurrentInst))
        {
            MyInst->speed = (val != 0) ? 200.0f : 0.0f;
        }
    }
}