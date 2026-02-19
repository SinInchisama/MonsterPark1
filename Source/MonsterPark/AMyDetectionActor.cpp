// Fill out your copyright notice in the Description page of Project Settings.


#include "AMyDetectionActor.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"

#include "FMonsterConditionFragment.h"

// Sets default values
AAMyDetectionActor::AAMyDetectionActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAMyDetectionActor::BeginPlay()
{
	Super::BeginPlay();
	
    UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (EntitySubsystem)
    {
        FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
        // 쿼리를 초기화합니다. (SharedPtr 형태의 매니저를 전달)
        // 헤더에 정의된 생성자 중 TSharedPtr<FMassEntityManager>를 받는 버전을 사용하게 됩니다.
        EnemyQuery = FMassEntityQuery(EntityManager.AsShared());
    }

    // 이제 초기화가 되었으므로 요구사항 추가가 가능합니다.
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
    float RadiusSq = FMath::Square(100.0f);

    EnemyQuery.ForEachEntityChunk(ExecContext, [this, MyLocation, RadiusSq](FMassExecutionContext& Context)
        {
            const int32 NumEntities = Context.GetNumEntities();
            TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
            TArrayView<FMonsterConditionFragment> Conditions = Context.GetMutableFragmentView<FMonsterConditionFragment>();
            //auto Transforms = Context.GetFragmentView<FTransformFragment>();

            for (int32 i = 0; i < NumEntities; ++i)
            {
                FVector EnemyLoc = Transforms[i].GetTransform().GetLocation();

                if (FVector::DistSquared(MyLocation, EnemyLoc) <= RadiusSq)     // 몬스터와 거리 계산
                {
                    Conditions[i].Damage += 100;                // if문 내부 지우고 애니메이션 시작 넣기
                    GetWorldTimerManager().SetTimer(
                        DetectionTimerHandle,
                        this,
                        &AAMyDetectionActor::OnResumeAction,
                        0.5f, // 0.5f
                        false     // 반복 여부: false
                    );
                    Attacking = false;
                }
            }
        }); 

       // UE_LOG(LogTemp, Warning, TEXT("Detected Count: %d"), Count);

}