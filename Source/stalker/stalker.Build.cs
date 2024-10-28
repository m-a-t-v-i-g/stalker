// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class stalker : ModuleRules
{
	public stalker(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GeneralMovement", "GameplayAbilities",
			"GameplayTasks", "GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Slate", "SlateCore", "UMG"
		});
		
		PublicIncludePaths.AddRange(new[]
		{
			"stalker/",
			"stalker/AbilitySystem",
			"stalker/InteractiveObjects",
			"stalker/InteractiveObjects/Items",
			"stalker/Interfaces",
			"stalker/Library",
			"stalker/Organic",
			"stalker/UI"
		});
	}
}
