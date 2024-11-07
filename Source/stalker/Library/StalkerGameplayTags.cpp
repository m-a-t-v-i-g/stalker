// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerGameplayTags.h"

namespace FStalkerGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move,					"InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_View,					"InputTag.View", "Look input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Inventory,				"InputTag.Inventory", "Inventory input.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_Sprint,				"Ability.Character.Sprint", "Sprint ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_Crouch,				"Ability.Character.Crouch",	"Crouch ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_Jump,					"Ability.Character.Jump", "Jump ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_BasicAction,			"Ability.Character.BasicAction", "Basic action ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_AlternativeAction,	"Ability.Character.AlternativeAction", "Alternative action ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_Reload,				"Ability.Character.Reload", "Reload ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_Interaction,			"Ability.Character.Interaction", "Interaction ability.");

	FGameplayTag FStalkerGameplayTags::FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}
}
