// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "InteractorInterface.h"
#include "ItemObject.h"
#include "ItemSystemCore.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

AItemActor::AItemActor()
{
	PhysicsRoot = CreateDefaultSubobject<USphereComponent>("Physics Root");
	SetRootComponent(PhysicsRoot);
	PhysicsRoot->CanCharacterStepUpOn = ECB_No;
	PhysicsRoot->SetCollisionProfileName("OverlapAllDynamic");

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	Mesh->SetupAttachment(GetRootComponent());
	Mesh->CanCharacterStepUpOn = ECB_No;
	Mesh->SetCollisionProfileName("Item");
	
	SetReplicates(true);
	SetReplicatingMovement(true);
}

void AItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ItemObject, COND_OwnerOnly);
}

void AItemActor::Destroyed()
{
	UnbindItemObject();
	Super::Destroyed();
}

bool AItemActor::OnInteract(AActor* Interactor)
{
	check(Interactor);
	
	if (ItemObject && ItemObject->IsInteractable())
	{
		if (auto InteractorInterface = Cast<IInteractorInterface>(Interactor))
		{
			InteractorInterface->InteractWithItem(ItemObject);
			return true;
		}
	}
	return false;
}

void AItemActor::BindItemObject(UItemObject* NewItemObject)
{
	if (!IsValid(NewItemObject))
	{
		return;
	}
	
	ItemObject = NewItemObject;
	OnBindItem();
}

void AItemActor::OnBindItem()
{
	UpdateItem();
}

void AItemActor::UnbindItemObject()
{
	if (!IsValid(ItemObject))
	{
		return;
	}

	UItemObject* PrevItemObject = ItemObject;
	ItemObject = nullptr;
	OnUnbindItem(PrevItemObject);
}

void AItemActor::OnUnbindItem(UItemObject* PrevItemObject)
{
	UpdateItem();
}

void AItemActor::UpdateItem()
{
}

void AItemActor::SetGrounded()
{
	PhysicsRoot->SetSimulatePhysics(true);
}

void AItemActor::SetEquipped()
{
	PhysicsRoot->SetSimulatePhysics(false);
}

void AItemActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (!IsBoundItem())
		{
			const UItemPredictedData* PreData = bUsePredictedData ? PredictedData : nullptr;
			if (UItemObject* NewItemObject = UItemSystemCore::GenerateItemObject(GetWorld(), ItemDefinition, PreData))
			{
				ItemObject = NewItemObject;
				ItemObject->BindItemActor(this);
			}
		}
	}
}

void AItemActor::OnRep_ItemObject(UItemObject* PrevItemObject)
{
	if (ItemObject)
	{
		OnBindItem();
	}
	else
	{
		OnUnbindItem(PrevItemObject);
	}
}
