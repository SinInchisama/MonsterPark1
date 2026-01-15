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
    "MassCommon",    // <--- 이 줄이 반드시 있어야 '외부 기호' 에러가 사라집니다.
    "MassMovement",
    "MassSpawner",
    "MassRepresentation",
    "MassNavigation"
});

        PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
