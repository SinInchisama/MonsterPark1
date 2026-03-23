#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "HeroChanceRow.generated.h"

USTRUCT(BlueprintType)
struct FHeroChanceRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 1코스트 확률
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost1;

	// 2코스트 확률
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost2;

	// 3코스트 확률
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost3;

	// 생성자 (기본값 설정)
	FHeroChanceRow()
		: Cost1(0.f), Cost2(0.f), Cost3(0.f)
	{
	}
};