// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class stalker : ModuleRules
{
	public stalker(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "GeneralMovement"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Slate", "SlateCore", "UMG"
		});
		
		PublicIncludePaths.AddRange(new[]
		{
			"stalker/Public"
		});

		PrivateIncludePaths.AddRange(new[]
		{
			"stalker/Private"
		});
	}
}
