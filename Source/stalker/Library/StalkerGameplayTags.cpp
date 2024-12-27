// Fill out your copyright notice in the Description page of Project Settings.

#include "StalkerGameplayTags.h"

namespace FStalkerGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move,					"InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_View,					"InputTag.View", "Look input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Inventory,				"InputTag.Inventory", "Inventory input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Slot,					"InputTag.Slot", "Slot input.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_Sprint,				"Ability.Sprint", "Sprint ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_Crouch,				"Ability.Crouch",	"Crouch ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_Jump,					"Ability.Jump", "Jump ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_Interaction,			"Ability.Interaction", "Interaction ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_Fire,					"Ability.Fire", "Weapon fire ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_FireAuto,				"Ability.FireAuto", "Weapon auto fire ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_MagReload,			"Ability.MagReload", "Magazine reload ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_SingleReload,			"Ability.SingleReload", "Single reload ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AbilityTag_Aiming,				"Ability.Aiming", "Weapon aiming ability.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(BlockAbilityTag_NoFiring,		"BlockAbility.NoFiring", "Block firing ability.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(EffectTag_BulletDamage,			"Effect.Damage.Bullet", "Bullet damage tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(EffectTag_BulletResistance,		"Effect.Resistance.Bullet", "Bullet resistance effect.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(EffectTag_BlastDamage,			"Effect.Damage.Blast", "Blast damage tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(EffectTag_BlastResistance,		"Effect.Resistance.Blast", "Blast resistance effect.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ArmorPartTag_Helmet,				"Character.Armor.Helmet", "Character helmet part.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ArmorPartTag_Body,				"Character.Armor.Body", "Character body part.");

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
