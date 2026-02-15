// 상태이상 및 데미지를 관리하는 프래그먼트
// 상태이상은 bool로 만들 예정

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "FMonsterConditionFragment.generated.h"

/**
 * 
 */
USTRUCT()
struct MONSTERPARK_API FMonsterConditionFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	float Damage = 0;

					
};
