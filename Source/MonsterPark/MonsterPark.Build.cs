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
    "MassActors",
    "GameplayAbilities",
    "GameplayTasks",
    "GameplayTags",
    "Niagara",
    "UMG",
    "Slate",
    "SlateCore",
    "Landscape"
});

        PrivateDependencyModuleNames.RemoveAll(x => x == "GameplayAbilities" || x == "GameplayTasks" || x == "GameplayTags");
	}
}
