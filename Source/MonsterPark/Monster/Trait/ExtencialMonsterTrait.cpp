// Fill out your copyright notice in the Description page of Project Settings.


#include "ExtencialMonsterTrait.h"

#include "MonsterPark/Monster/Fragment/FMonsterStatusFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterConditionFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterSharedFragment.h"
#include "MonsterPark/Monster/Fragment/FMonsterRandomMoveFragment.h"

#include "MassEntityTemplateRegistry.h"


void UExtencialMonsterTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMonsterStatusFragment& StatusFrag = BuildContext.AddFragment_GetRef<FMonsterStatusFragment>();
	StatusFrag = MonstStatusParams;

	FMonsterConditionFragment& ConditionFrag = BuildContext.AddFragment_GetRef<FMonsterConditionFragment>();

	FMonsterRandomMoveFragment& RandomMoveFrag = BuildContext.AddFragment_GetRef<FMonsterRandomMoveFragment>();

	FMonsterRoundSharedFragment RoundSharedData;
	RoundSharedData.HitEffect = HitEffect;
	BuildContext.AddSharedFragment(FSharedStruct::Make(RoundSharedData));
}
