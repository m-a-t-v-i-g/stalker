// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "ItemObject.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

FName AItemActor::PhysicCollisionName {"Physic Collision"};
FName AItemActor::InteractionSphereName {"Interaction Sphere"};

AItemActor::AItemActor()
{
	PhysicCollision = CreateDefaultSubobject<USphereComponent>(PhysicCollisionName);
	SetRootComponent(PhysicCollision);
	PhysicCollision->SetCollisionEnabled(ECollisionEnabled::Type::ProbeOnly);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(InteractionSphereName);
	InteractionSphere->SetupAttachment(GetRootComponent());
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	Mesh->SetupAttachment(GetRootComponent());
	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AItemActor, ItemObject, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AItemActor, bHanded,	COND_OwnerOnly)
}

void AItemActor::Destroyed()
{
	UnbindItemObject();
	
	Super::Destroyed();
}

void AItemActor::BindItemObject(UItemObject* NewItemObject)
{
	if (!IsValid(NewItemObject))
	{
		return;
	}
	
	ItemObject = NewItemObject;
	ItemObject->BindItemActor(this);
	
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

	ItemObject->UnbindItemActor();
	ItemObject = nullptr;
	
	OnUnbindItem();
}

void AItemActor::OnUnbindItem()
{
	UpdateItem();
}

void AItemActor::SetEquipped()
{
	bHanded = true;
	PhysicCollision->SetSimulatePhysics(false);
}

void AItemActor::SetGrounded()
{
	bHanded = false;
	PhysicCollision->SetSimulatePhysics(true);
}

void AItemActor::UpdateItem()
{

}

void AItemActor::OnRep_ItemObject()
{
	if (ItemObject)
	{
		OnBindItem();
	}
}

void AItemActor::OnRep_Handed()
{
	bHanded ? SetEquipped() : SetGrounded();
}
