// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemObject.h"
#include "ItemActor.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

void UItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ItemData,		COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, ItemDefinition,	COND_OwnerOnly);
}

bool UItemInstance::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = false;
	return bReplicateSomething;
}

void UItemInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition,
                                    const UItemPredictedData* PredictedData)
{
	ItemData.ItemId = NewItemId;

	if (Definition)
	{
		ItemDefinition = Definition;
		
		if (PredictedData)
		{
			ItemData.Amount = PredictedData->Amount;
			ItemData.Endurance = PredictedData->Endurance;
		}
	}
}

void UItemInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance)
{
	ItemData.ItemId = NewItemId;

	if (Definition)
	{
		ItemDefinition = Definition;
		
		if (Instance)
		{
			ItemData.Amount = Instance->ItemData.Amount;
			ItemData.Endurance = Instance->ItemData.Endurance;
		}
	}
}

void UItemObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UItemObject, ItemDefinition,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UItemObject, ItemInstance,		COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UItemObject, BoundItemActor,	COND_OwnerOnly);
}

bool UItemObject::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bReplicateSomething = false;
	
	if (ItemInstance)
	{
		bReplicateSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		bReplicateSomething |= ItemInstance->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}
	
	return bReplicateSomething;
}

void UItemObject::Use_Implementation(UObject* Source)
{
	IUsableInterface::Use_Implementation(Source);
}

void UItemObject::InitItem(const uint32 ItemId, const UItemObject* ItemObject)
{
	ItemDefinition = ItemObject->ItemDefinition;
	ensure(ItemDefinition);

	if (!GetDefinition())
	{
		return;
	}
	
	UItemInstance* NewItemInstance = NewObject<UItemInstance>(this, GetDefinition()->ItemInstanceClass,
	                                                          FName(GetScriptName().ToString() + FString::Printf(
		                                                          TEXT("_instance%d"), ItemId)));
	if (!NewItemInstance)
	{
		return;
	}
	
	ItemInstance = NewItemInstance;
	ItemInstance->SetupProperties(ItemId, GetDefinition(), ItemObject->GetItemInstance());
}

void UItemObject::InitItem(const uint32 ItemId, const UItemDefinition* Definition, const UItemPredictedData* PredictedData)
{
	ItemDefinition = Definition;
	ensure(ItemDefinition);
	
	if (!GetDefinition())
	{
		return;
	}

	UItemInstance* NewItemInstance = NewObject<UItemInstance>(this, GetDefinition()->ItemInstanceClass,
	                                                          FName(GetScriptName().ToString() + FString::Printf(
		                                                          TEXT("_instance%d"), ItemId)));
	if (!NewItemInstance)
	{
		return;
	}

	ItemInstance = NewItemInstance;
	ItemInstance->SetupProperties(ItemId, GetDefinition(), PredictedData);
}

void UItemObject::BindItemActor(AItemActor* BindItem)
{
	if (!IsValid(BindItem))
	{
		return;
	}

	BoundItemActor = BindItem;
	BoundItemActor->BindItemObject(this);
	OnBindItemActor();
}

void UItemObject::OnBindItemActor()
{
}

void UItemObject::UnbindItemActor()
{
	if (!IsValid(BoundItemActor))
	{
		return;
	}

	AItemActor* PrevItemActor = BoundItemActor;
	BoundItemActor = nullptr;
	OnUnbindItemActor(PrevItemActor);
}

void UItemObject::OnUnbindItemActor(AItemActor* PrevItemActor)
{
}

void UItemObject::SetAmount(uint32 Amount) const
{
	if (ItemInstance)
	{
		ItemInstance->ItemData.Amount = Amount;
	}
}

void UItemObject::AddAmount(uint32 Amount) const
{
	if (ItemInstance)
	{
		ItemInstance->ItemData.Amount += Amount;
	}
}

void UItemObject::RemoveAmount(uint32 Amount) const
{
	if (ItemInstance)
	{
		ItemInstance->ItemData.Amount -= Amount;
	}
}

bool UItemObject::CanCollected() const
{
	return IsGrounded() && !IsCollected() && !IsEquipped();
}

bool UItemObject::CanStackItem(const UItemObject* OtherItem) const
{
	return this != OtherItem && IsStackable() && IsSimilar(OtherItem);
}

bool UItemObject::IsSimilar(const UItemObject* OtherItemObject) const
{
	bool bResult = GetDefinition() == OtherItemObject->GetDefinition();
	if (bResult)
	{
		bResult &= GetEndurance() == OtherItemObject->GetEndurance();
	}
	return bResult;
}

bool UItemObject::HasBoundActor() const
{
	return IsValid(BoundItemActor);
}

uint32 UItemObject::GetItemId() const
{
	return ItemInstance->ItemData.ItemId;
}

uint16 UItemObject::GetAmount() const
{
	return ItemInstance->ItemData.Amount;
}

float UItemObject::GetEndurance() const
{
	return ItemInstance->ItemData.Endurance;
}

EItemMode UItemObject::GetItemMode() const
{
	return ItemInstance->ItemData.Mode;
}

void UItemObject::SetGrounded()
{
	ItemInstance->ItemData.Mode = EItemMode::Grounded;
}

void UItemObject::SetCollected()
{
	ItemInstance->ItemData.Mode = EItemMode::Collected;
}

void UItemObject::SetEquipped()
{
	ItemInstance->ItemData.Mode = EItemMode::Equipped;
}

bool UItemObject::IsGrounded() const
{
	return GetItemMode() == EItemMode::Grounded;
}

bool UItemObject::IsCollected() const
{
	return GetItemMode() == EItemMode::Collected;
}

bool UItemObject::IsEquipped() const
{
	return GetItemMode() == EItemMode::Equipped;
}

const UItemDefinition* UItemObject::GetDefinition() const
{
	return ItemDefinition;
}

FName UItemObject::GetScriptName() const
{
	return GetDefinition()->ScriptName;
}

FGameplayTag UItemObject::GetItemTag() const
{
	return GetDefinition()->Tag;
}

UClass* UItemObject::GetActorClass() const
{
	return GetDefinition()->ItemActorClass;
}

UClass* UItemObject::GetObjectClass() const
{
	return GetDefinition()->ItemObjectClass;
}

FText UItemObject::GetItemName() const
{
	return GetDefinition()->Name;
}

FText UItemObject::GetItemDesc() const
{
	return GetDefinition()->Description;
}

UTexture2D* UItemObject::GetThumbnail() const
{
	return GetDefinition()->Thumbnail.LoadSynchronous();
}

FIntPoint UItemObject::GetItemSize() const
{
	return GetDefinition()->Size;
}

bool UItemObject::IsUsable() const
{
	return GetDefinition()->bUsable;
}

bool UItemObject::IsDroppable() const
{
	return GetDefinition()->bDroppable;
}

bool UItemObject::IsStackable() const
{
	return GetDefinition()->bStackable;
}

uint32 UItemObject::GetStackAmount() const
{
	return GetDefinition()->StackAmount;
}

AItemActor* UItemObject::GetBoundActor() const
{
	return BoundItemActor;
}

UItemInstance* UItemObject::GetItemInstance() const
{
	return ItemInstance;
}

FTimerManager& UItemObject::GetWorldTimerManager() const
{
	return GetWorld()->GetTimerManager();
}

void UItemObject::OnRep_BoundItem(AItemActor* PrevItemActor)
{
	if (BoundItemActor)
	{
		OnBindItemActor();
	}
	else
	{
		OnUnbindItemActor(PrevItemActor);
	}
}
