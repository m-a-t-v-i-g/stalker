// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySet.h"
#include "Abilities/GameplayAbility.h"
#include "Components/OrganicAbilityComponent.h"

UAbilitySet::UAbilitySet(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC, TArray<FGameplayAbilitySpecHandle>& OutHandles,
                                      UObject* SourceObject) const
{
	check(ASC);

	if (ASC->IsOwnerActorAuthoritative())
	{
		for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
		{
			const FAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];
			if (!IsValid(AbilityToGrant.Ability))
			{
				continue;
			}

			UGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UGameplayAbility>();
			FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
			AbilitySpec.SourceObject = SourceObject;
			AbilitySpec.DynamicAbilityTags.AddTag(AbilityToGrant.InputTag);

			FGameplayAbilitySpecHandle OutSpec = ASC->GiveAbility(AbilitySpec);
			OutHandles.Add(OutSpec);
		}
	}
}
