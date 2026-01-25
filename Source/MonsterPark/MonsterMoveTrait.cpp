// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterMoveTrait.h"
#include "FMonsterTargetFragment.h"
#include "MassEntityTemplateRegistry.h"

void UMonsterMoveTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{

	FMonsterTargetFragment& TargetFrag =
		BuildContext.AddFragment_GetRef<FMonsterTargetFragment>();

	TargetFrag.Target = FVector(-1080, 1080, 60.f);
	/*FMonsterTargetFragment InitialData;
	InitialData.Target = FVector(-1080,
		1080,
		60.f);*/

	//BuildContext.AddFragment<FMonsterTargetFragment>();
}