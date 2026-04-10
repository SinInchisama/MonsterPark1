// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterMoveTrait.h"
#include "MonsterPark/Monster/Fragment/FMonsterTargetFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterConditionFragment.h"
#include "MassEntityTemplateRegistry.h"
#include "MonsterPark/Monster/Tag/MonsterSpawnTag.h"
#include "MonsterPark/Monster/Fragment/FMonsterSharedFragment.h"

void UMonsterMoveTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{

	FMonsterTargetFragment& TargetFrag =
		BuildContext.AddFragment_GetRef<FMonsterTargetFragment>();													// 현재 생성 주기와 이동 위치를 관리하는 프래그먼트

	FMonsterStatusFragment& StatusFrag = BuildContext.AddFragment_GetRef<FMonsterStatusFragment>();					// 체력 및 방어력을 관리할 Fragment
	StatusFrag = MonstStatusParams;																					// 언리얼 내부에서 관리 가능

	FMonsterConditionFragment& ConditionFrag = BuildContext.AddFragment_GetRef<FMonsterConditionFragment>();		// Damage 및 상태이상을 관리할 Fragment

	BuildContext.AddTag<FMonsterSpawnTag>();

	FMonsterRoundSharedFragment RoundSharedData;
	RoundSharedData.HitEffect = HitEffect;
	BuildContext.AddSharedFragment(FSharedStruct::Make(RoundSharedData));

	TargetFrag.Target = FVector(-2600, 400, 60.f);
	
	//BuildContext.AddFragment<FMonsterTargetFragment>();
}