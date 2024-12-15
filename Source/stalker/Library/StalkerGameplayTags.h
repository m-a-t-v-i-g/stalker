// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace FStalkerGameplayTags
{
	STALKER_API	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_View);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Inventory);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Slot);

	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AbilityTag_Sprint);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AbilityTag_Crouch);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AbilityTag_Jump);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AbilityTag_BasicAction);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AbilityTag_AlternativeAction);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AbilityTag_Reload);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AbilityTag_Interaction);

	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(EffectTag_Damage);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(EffectTag_Heal);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(EffectTag_BulletResistance);
	STALKER_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(EffectTag_BlastResistance);
}
