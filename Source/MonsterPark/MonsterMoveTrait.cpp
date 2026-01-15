// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterMoveTrait.h"
#include "FMonsterTargetFragment.h"
#include "MassEntityTemplateRegistry.h"

void UMonsterMoveTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMonsterTargetFragment InitialData;
	InitialData.Target = FVector(FMath::RandRange(-1000.f, 1000.f), FMath::RandRange(-1000.f, 1000.f), 0.f);

	BuildContext.AddFragment<FMonsterTargetFragment>();
}