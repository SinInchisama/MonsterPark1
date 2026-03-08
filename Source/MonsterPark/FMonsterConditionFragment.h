// �����̻� �� �������� �����ϴ� �����׸�Ʈ
// �����̻��� bool�� ���� ����

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

	UPROPERTY(EditAnywhere, Category = "Stats")
	float SpeedMultiplier = 1.0f;
};
