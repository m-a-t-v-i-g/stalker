// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class stalker : ModuleRules
{
	public stalker(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GeneralMovement",
			"ModularGameplay", "GameplayAbilities", "GameplayTasks", "GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Slate", "SlateCore", "UMG", "CommonUI", "Niagara"
		});
		
		PublicIncludePaths.AddRange(new[]
		{
			"stalker/",
			"stalker/AbilitySystem",
			"stalker/Character",
			"stalker/Character/Components",
			"stalker/Character/Data",
			"stalker/DamageSystem",
			"stalker/DamageSystem/Types",
			"stalker/Input",
			"stalker/InteractiveObjects",
			"stalker/InteractiveObjects/Components",
			"stalker/Interfaces",
			"stalker/Inventory",
			"stalker/Inventory/Components",
			"stalker/ItemSystem",
			"stalker/Library",
			"stalker/Mobs",
			"stalker/Organic",
			"stalker/PhysicalObjects",
			"stalker/Player",
			"stalker/UI",
			"stalker/UI/Character",
			"stalker/UI/HUD",
			"stalker/UI/HUD/Components",
			"stalker/UI/Inventory"
		});
	}
}
