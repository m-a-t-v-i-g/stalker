// Fill out your copyright notice in the Description page of Project Settings.

#include "ArmorObject.h"
#include "ArmorActor.h"
#include "GameplayEffect.h"
#include "ItemSystemCore.h"
#include "Kismet/KismetMathLibrary.h"
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
	if (auto ArmorDefinition = Cast<UArmorDefinition>(Definition))
	{
		if (auto ArmorPredictedData = Cast<UArmorPredictedData>(PredictedData))
		{
			// Predicted data
		}
	}

	Super::SetupProperties(NewItemId, Definition, PredictedData);
}

void UArmorInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance)
{	
	if (auto ArmorInstance = Cast<UArmorInstance>(Instance))
	{

	}

	Super::SetupProperties(NewItemId, Definition, Instance);
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
	UpdateProtectionModifiers();
	
	Super::OnEnduranceUpdated(NewEndurance, PrevEndurance);
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

void UArmorObject::UpdateProtectionModifiers() const
{
	if (FArmorStaticDataTableRow* ArmorProperties = GetArmorProperties())
	{
		TMap<FGameplayTag, float> NewModifiers;
		const TMap<FGameplayTag, float>& ArmorModifiers = ArmorProperties->ProtectionModifiers;
		UCurveFloat* ProtectionCurve = ArmorProperties->ProtectionFactor;

		for (const auto& Modifier : ArmorModifiers)
		{
			float NormalizedEndurance = GetEndurance() / 100.0f;
			float FinalModifierValue = Modifier.Value;

			if (ProtectionCurve)
			{
				FinalModifierValue *= ProtectionCurve->GetFloatValue(NormalizedEndurance);
			}
			else
			{
				FinalModifierValue = UKismetMathLibrary::MapRangeClamped(
					NormalizedEndurance, 0.0f, 1.0f, 0.0f, Modifier.Value);
			}

			NewModifiers.Add(Modifier.Key, FinalModifierValue);
		}

		GetArmorInstance()->ArmorData.ProtectionModifiers = NewModifiers;
	}
}

const TMap<FGameplayTag, float>& UArmorObject::GetProtectionModifiers() const
{
	return GetArmorInstance()->ArmorData.ProtectionModifiers;
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

FArmorStaticDataTableRow* UArmorObject::GetArmorProperties() const
{
	if (UDataTable* ArmorPropertiesDT = GetArmorDefinition()->ArmorPropertiesDataTable)
	{
		if (FArmorStaticDataTableRow* Row = ArmorPropertiesDT->FindRow<FArmorStaticDataTableRow>(GetScriptName(), ""))
		{
			return Row;
		}
	}
	return nullptr;
}
