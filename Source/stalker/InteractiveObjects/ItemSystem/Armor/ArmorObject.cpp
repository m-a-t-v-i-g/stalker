// Fill out your copyright notice in the Description page of Project Settings.

#include "ArmorObject.h"
#include "ArmorActor.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"

void UArmorInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ThisClass, ArmorData, COND_OwnerOnly);
}

bool UArmorInstance::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	return bReplicateSomething;
}

void UArmorInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
                                     const UItemPredictedData* PredictedData)
{
	Super::SetupProperties(NewItemId, Definition, PredictedData);

	if (auto ArmorDefinition = Cast<UArmorDefinition>(Definition))
	{
		// Logic
		
		if (auto ArmorPredictedData = Cast<UArmorPredictedData>(PredictedData))
		{
			// Predicted data
		}
	}
}

void UArmorInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance)
{
	Super::SetupProperties(NewItemId, Definition, Instance);
	
	if (auto ArmorInstance = Cast<UArmorInstance>(Instance))
	{
		// Logic
	}
}

void UArmorObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UArmorObject::Use_Implementation(UObject* Source)
{
	Super::Use_Implementation(Source);
	
	// TODO: try use item
}

void UArmorObject::OnEnduranceUpdated(float NewEndurance, float PrevEndurance)
{
	Super::OnEnduranceUpdated(NewEndurance, PrevEndurance);

	UpdateModifiers();
}

void UArmorObject::OnBindItemActor()
{
	Super::OnBindItemActor();
}

void UArmorObject::OnUnbindItemActor(AItemActor* PrevItemActor)
{
	Super::OnUnbindItemActor(PrevItemActor);
}

bool UArmorObject::IsCorrespondsTo(const UItemObject* OtherItemObject) const
{
	return Super::IsCorrespondsTo(OtherItemObject);
}

void UArmorObject::UpdateModifiers() const
{
	TMap<FGameplayTag, float> NewModifiers;
	const TMap<FGameplayTag, float>& ArmorModifiers = GetModifiers();
	UCurveFloat* ProtectionCurve = GetProtectionFactorCurve();
	
	for (const auto& Modifier : ArmorModifiers)
	{
		float FinalModifierValue = Modifier.Value;
		if (ProtectionCurve)
		{
			FinalModifierValue *= ProtectionCurve->GetFloatValue(GetEndurance() / 100.0f);
		}

		NewModifiers.Add(Modifier.Key, FinalModifierValue);
	}

	GetArmorInstance()->ArmorData.Modifiers = NewModifiers;
}

const UArmorDefinition* UArmorObject::GetArmorDefinition() const
{
	return Cast<UArmorDefinition>(GetDefinition());
}

USkeletalMesh* UArmorObject::GetVisual() const
{
	return nullptr; // TODO;
}

AArmorActor* UArmorObject::GetArmorActor() const
{
	return GetBoundActor<AArmorActor>();
}

UArmorInstance* UArmorObject::GetArmorInstance() const
{
	return GetItemInstance<UArmorInstance>();
}

UCurveFloat* UArmorObject::GetProtectionFactorCurve() const
{
	return GetArmorDefinition()->ProtectionFactorCurve;
}

UClass* UArmorObject::GetArmorEffect() const
{
	return GetArmorDefinition()->ArmorEffect;
}

const TMap<FGameplayTag, float>& UArmorObject::GetModifiers() const
{
	return GetArmorDefinition()->Modifiers;
}
