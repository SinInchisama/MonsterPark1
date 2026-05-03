#include "MonsterReplicationTypes.h"
#include "MassEntityManager.h"
#include "MassRepresentationFragments.h"
#include "MassCommonFragments.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h" 

void FMonsterClientBubbleHandler::PostReplicatedChangeEntity(const FMassEntityView& EntityView, const FMonsterReplicatedAgent& ReplicatedAgent)
{
    FTransformFragment& TransformFragment = EntityView.GetFragmentData<FTransformFragment>();

    TransformFragment.GetMutableTransform().SetLocation(ReplicatedAgent.Location);
}

void FMonsterClientBubbleHandler::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSectionsCount)
{
    // 1. 데이터를 담을 뷰 선언
    TArrayView<FTransformFragment> TransformFragments;

    // 2. 쿼리 요구사항 설정 람다
    auto AddRequirementsForSpawnQuery = [this](FMassEntityQuery& InQuery)
        {
            InQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
            // 블로그 로직에 따라 추가적인 Fragment가 필요하면 여기에 등록합니다.
        };

    // 3. 실행 컨텍스트에서 뷰를 캐싱하는 람다
    auto CacheFragmentViewsForSpawnQuery = [&](FMassExecutionContext& InExecContext)
        {
            TransformFragments = InExecContext.GetMutableFragmentView<FTransformFragment>();
        };

    // 4. 각 엔티티별 데이터를 세팅하는 람다
    auto SetSpawnedEntityData = [&](const FMassEntityView& EntityView, const FMonsterReplicatedAgent& ReplicatedAgent, const int32 EntityIdx)
        {
            // ReplicatedAgent에서 위치 정보를 가져와 TransformFragment에 꽂아넣습니다.
            TransformFragments[EntityIdx].GetMutableTransform().SetLocation(ReplicatedAgent.Location);

            // 블로그 예제처럼 Yaw 값을 압축해서 보냈다면 해제하여 적용합니다.
            const float DecodedYaw = ReplicatedAgent.Yaw * (360.f / 256.f);
            TransformFragments[EntityIdx].GetMutableTransform().SetRotation(FRotator(0.f, DecodedYaw, 0.f).Quaternion());
        };

    // 5. 블로그의 핵심: PostReplicatedAddHelper 호출
    // 이 함수가 내부적으로 엔티티를 스폰하고 위 람다들을 실행합니다.
    PostReplicatedAddHelper(
        AddedIndices,
        AddRequirementsForSpawnQuery,
        CacheFragmentViewsForSpawnQuery,
        SetSpawnedEntityData,
        // 여기를 이렇게 바꾸세요!
        [this](const FMassEntityView& EntityView, const FMonsterReplicatedAgent& ReplicatedAgent)
        {
            PostReplicatedChangeEntity(EntityView, ReplicatedAgent);
        }
    );
}

void FMonsterClientBubbleHandler::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSectionsCount)
{
    PostReplicatedChangeHelper(ChangedIndices, [this](const FMassEntityView& EntityView, const FMonsterReplicatedAgent& ReplicatedAgent)
        {
            PostReplicatedChangeEntity(EntityView, ReplicatedAgent);
        });
}