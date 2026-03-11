// Fill out your copyright notice in the Description page of Project Settings.


#include "Shielder.h"
#include "MonsterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "FMonsterConditionFragment.h" // 엔티티 상태 조각
#include "MassCommonFragments.h"      // Transform 등 공통 조각

void AShielder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
};

UAnimMontage* AShielder::GetDetectedMontage() const
{
    // 부모 클래스의 PlayDetectedMontageIfNeeded()가 이 몽타주를 사용하게 됩니다.
    return ShielderFullMontage;
}

void AShielder::FindEnemiesInArea()
{
    // 1. 부모의 공통 로직 실행 (적 감지 시 bEnemyDetected = true 및 데미지 적용)
    Super::FindEnemiesInArea();

    // 2. Shielder만의 특수 로직 (주변 적 슬로우) 추가
    UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    if (!EntitySubsystem || !AbilitySystemComponent) return;

    const float RangeValue = AbilitySystemComponent->GetNumericAttribute(UMonsterAttributeSet::GetRangeAttribute());
    float RadiusSq = FMath::Square(RangeValue);
    FVector MyLocation = GetActorLocation();

    FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
    FMassExecutionContext ExecContext(EntityManager, 0.0f);

    // 슬로우 효과 부여 (Mass 쿼리 재사용)
    EnemyQuery.ForEachEntityChunk(ExecContext, [this, MyLocation, RadiusSq](FMassExecutionContext& Context)
        {
            const int32 NumEntities = Context.GetNumEntities();
            auto Transforms = Context.GetMutableFragmentView<FTransformFragment>();
            auto Conditions = Context.GetMutableFragmentView<FMonsterConditionFragment>();

            for (int32 i = 0; i < NumEntities; ++i)
            {
                if (FVector::DistSquared(MyLocation, Transforms[i].GetTransform().GetLocation()) <= RadiusSq)
                {
                    // 적 발견 시 속도 저하 적용
                    Conditions[i].SpeedMultiplier = SlowMultiplier;
                }
                else
                {
                    // 범위를 벗어난 적은 속도 복구
                    Conditions[i].SpeedMultiplier = 1.0f;
                }
            }
        });
}

void AShielder::PlayDetectedMontageIfNeeded()
{
    if (!bEnemyDetected) return;

    if (USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>())
    {
        if (UAnimInstance* AnimInst = Mesh->GetAnimInstance())
        {
            if (ShielderFullMontage && !AnimInst->Montage_IsPlaying(ShielderFullMontage))
            {
                // 1. 몽타주를 일단 실행
                AnimInst->Montage_Play(ShielderFullMontage);

                if (!bHasPlayedPassive)
                {
                    // 처음 감지 시: Passive 섹션부터 재생 (기본값이면 생략 가능)
                    AnimInst->Montage_JumpToSection(FName("Passive"), ShielderFullMontage);
                    bHasPlayedPassive = true; // 이제 패시브 실행됨을 기록
                    UE_LOG(LogTemp, Log, TEXT("Shielder: Playing Passive for the first time."));
                }
                else
                {
                    // 두 번째 감지부터: 바로 Attack 섹션으로 점프
                    AnimInst->Montage_JumpToSection(FName("Attack"), ShielderFullMontage);
                    UE_LOG(LogTemp, Log, TEXT("Shielder: Playing Attack section."));
                }
            }
        }
    }
}