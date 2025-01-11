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

void UItemInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemPredictedData* PredictedData)
{
	FItemInstanceData PrevItemData = ItemData;
	
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
	
	UpdateItemInstance(PrevItemData);
}

void UItemInstance::SetupProperties(uint32 NewItemId, const UItemDefinition* Definition, const UItemInstance* Instance)
{
	FItemInstanceData PrevItemData = ItemData;
	
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

	UpdateItemInstance(PrevItemData);
}

void UItemInstance::UpdateItemInstance(const FItemInstanceData& PrevItemData)
{
	if (OnItemDataChangedDelegate.IsBound())
	{
		OnItemDataChangedDelegate.Execute(ItemData, PrevItemData);	
	}
}

void UItemInstance::OnRep_ItemData(const FItemInstanceData& PrevItemData)
{
	UpdateItemInstance(PrevItemData);
}

UItemObject::UItemObject(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
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
	ItemInstance->OnItemDataChangedDelegate.BindUObject(this, &UItemObject::OnItemInstanceDataChanged);
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
	ItemInstance->OnItemDataChangedDelegate.BindUObject(this, &UItemObject::OnItemInstanceDataChanged);
	ItemInstance->SetupProperties(ItemId, GetDefinition(), PredictedData);
}

void UItemObject::OnItemInstanceDataChanged(const FItemInstanceData& ItemData, const FItemInstanceData& PrevItemData)
{
	UpdateEndurance(ItemData.Endurance, PrevItemData.Endurance);
	UpdateAmount(ItemData.Amount, PrevItemData.Amount);
	UpdateMode(ItemData.Mode, PrevItemData.Mode);
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
	BoundItemActor->UnbindItemObject();
	BoundItemActor = nullptr;
	OnUnbindItemActor(PrevItemActor);
}

void UItemObject::OnUnbindItemActor(AItemActor* PrevItemActor)
{
}

void UItemObject::SetAmount(uint32 Amount)
{
	if (ItemInstance)
	{
		uint32 PrevAmount = ItemInstance->ItemData.Amount;
		ItemInstance->ItemData.Amount = Amount;
		UpdateAmount(Amount, PrevAmount);
	}
}

void UItemObject::AddAmount(uint32 Amount)
{
	if (ItemInstance)
	{
		uint32 PrevAmount = ItemInstance->ItemData.Amount;
		uint32 NewAmount = ItemInstance->ItemData.Amount + Amount;
		ItemInstance->ItemData.Amount = NewAmount;
		UpdateAmount(NewAmount, PrevAmount);
	}
}

void UItemObject::RemoveAmount(uint32 Amount)
{
	if (ItemInstance)
	{
		uint32 PrevAmount = ItemInstance->ItemData.Amount;
		uint32 NewAmount = ItemInstance->ItemData.Amount - Amount;
		ItemInstance->ItemData.Amount = NewAmount;
		UpdateAmount(NewAmount, PrevAmount);
	}
}

void UItemObject::UpdateAmount(uint32 NewAmount, uint32 PrevAmount)
{
	OnAmountUpdated(NewAmount, PrevAmount);
	OnAmountChangeDelegate.Broadcast(NewAmount);
}

void UItemObject::OnAmountUpdated(uint32 NewAmount, uint32 PrevAmount)
{
}

void UItemObject::RecoveryEndurance()
{
	if (ItemInstance)
	{
		float PrevEndurance = ItemInstance->ItemData.Endurance / 100.0f;
		float NewEndurance = ItemInstance->ItemData.Endurance = 100.0f;
		UpdateEndurance(NewEndurance, PrevEndurance);
	}
}

void UItemObject::SpoilEndurance(const FGameplayTag& DamageTag, float DamageValue)
{
	if (ItemInstance)
	{
		if (const float* SpoilModifierPtr = GetSpoilModifiers().Find(DamageTag))
		{
			const float SpoilModifier = *SpoilModifierPtr;
			if (FMath::IsNearlyZero(SpoilModifier))
			{
				return;
			}

			float PrevEndurance = ItemInstance->ItemData.Endurance / 100.0f;
			float NewEndurance = PrevEndurance - SpoilModifier * DamageValue;
			ItemInstance->ItemData.Endurance = NewEndurance * 100.0f;
			UpdateEndurance(NewEndurance, PrevEndurance);
		}
	}
}

void UItemObject::UpdateEndurance(float NewEndurance, float PrevEndurance)
{
	OnEnduranceUpdated(NewEndurance, PrevEndurance);
	OnEnduranceChangeDelegate.Broadcast(NewEndurance);
}

void UItemObject::OnEnduranceUpdated(float NewEndurance, float PrevEndurance)
{
}

bool UItemObject::IsInteractable() const
{
	return IsGrounded() && !IsCollected() && !IsEquipped();
}

bool UItemObject::CanStackItem(const UItemObject* OtherItem) const
{
	return this != OtherItem && IsStackable() && IsCorrespondsTo(OtherItem);
}

bool UItemObject::IsCorrespondsTo(const UItemObject* OtherItemObject) const
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
	if (ItemInstance)
	{
		EItemMode PrevMode = ItemInstance->ItemData.Mode;
		EItemMode NewMode = EItemMode::Grounded;
		ItemInstance->ItemData.Mode = NewMode;
		UpdateMode(NewMode, PrevMode);
	}
}

void UItemObject::SetCollected()
{
	if (ItemInstance)
	{
		EItemMode PrevMode = ItemInstance->ItemData.Mode;
		EItemMode NewMode = EItemMode::Collected;
		ItemInstance->ItemData.Mode = NewMode;
		UpdateMode(NewMode, PrevMode);
	}
}

void UItemObject::SetEquipped()
{
	if (ItemInstance)
	{
		EItemMode PrevMode = ItemInstance->ItemData.Mode;
		EItemMode NewMode = EItemMode::Equipped;
		ItemInstance->ItemData.Mode = NewMode;
		UpdateMode(NewMode, PrevMode);
	}
}

void UItemObject::UpdateMode(EItemMode NewMode, EItemMode PrevMode)
{
	OnModeUpdated(NewMode, PrevMode);
	OnModeChangeDelegate.Broadcast(NewMode);
}

void UItemObject::OnModeUpdated(EItemMode NewMode, EItemMode PrevMode)
{
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

bool UItemObject::IsSpoiling() const
{
	return true;
}

uint32 UItemObject::GetStackAmount() const
{
	return GetDefinition()->StackAmount;
}

const TMap<FGameplayTag, float>& UItemObject::GetSpoilModifiers() const
{
	return GetDefinition()->SpoilModifiers;
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

void UItemObject::OnRep_ItemInstance(UItemInstance* PrevItemInstance)
{
	if (PrevItemInstance)
	{
		PrevItemInstance->OnItemDataChangedDelegate.Unbind();
	}

	if (ItemInstance)
	{
		OnItemInstanceDataChanged(ItemInstance->ItemData,
		                          PrevItemInstance ? PrevItemInstance->ItemData : FItemInstanceData());
		ItemInstance->OnItemDataChangedDelegate.BindUObject(this, &UItemObject::OnItemInstanceDataChanged);
	}
}

void UItemObject::OnRep_BoundItem(AItemActor* PrevItemActor)
{
	if (PrevItemActor)
	{
		OnUnbindItemActor(PrevItemActor);
	}
	
	if (BoundItemActor)
	{
		OnBindItemActor();
	}
}
