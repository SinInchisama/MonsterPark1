// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MonsterPark : ModuleRules
{
	public MonsterPark(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "CoreUObject", "Engine", "InputCore",
    "MassEntity",
    "MassCommon",    // <--- �� ���� �ݵ�� �־�� '�ܺ� ��ȣ' ������ ������ϴ�.
    "MassMovement",
    "MassSpawner",
    "MassRepresentation",
    "MassNavigation",
    "GameplayAbilities",
    "GameplayTasks",
    "GameplayTags",
    "UMG"
});

        PrivateDependencyModuleNames.RemoveAll(x => x == "GameplayAbilities" || x == "GameplayTasks" || x == "GameplayTags");
	}
}
