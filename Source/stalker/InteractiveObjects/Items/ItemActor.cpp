// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "ItemObject.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

FName AItemActor::SphereComponentName {"ItemCollision"};

AItemActor::AItemActor()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(SphereComponentName);
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::Type::ProbeOnly);

	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>("PreviewMesh");
	PreviewMesh->SetupAttachment(GetRootComponent());
	
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

void AItemActor::InitializeItem(UItemObject* NewItemObject)
{
	if (!IsValid(NewItemObject)) return;
	
	ItemObject = NewItemObject;
	ItemObject->BindItem(this);
	OnInitializeItem();
}

void AItemActor::OnInitializeItem()
{
	UpdateItem();
}

void AItemActor::UnbindItem()
{
	if (!IsValid(ItemObject)) return;

	ItemObject->UnbindItem();
	ItemObject = nullptr;
	OnUnbindItem();
}

void AItemActor::OnUnbindItem()
{
}

void AItemActor::SetHandedMode()
{
	bHanded = true;
	SphereComponent->SetSimulatePhysics(false);

	if (PreviewMesh->GetStaticMesh())
	{
		PreviewMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	}
	
	if (Mesh->GetSkeletalMeshAsset())
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	}
	
	if (bHidePreviewMeshWhenHanded)
	{
		PreviewMesh->SetHiddenInGame(true);
	}
}

void AItemActor::SetFreeMode()
{
	bHanded = false;
	SphereComponent->SetSimulatePhysics(true);

	if (PreviewMesh->GetStaticMesh())
	{
		PreviewMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	}
	
	if (Mesh->GetSkeletalMeshAsset())
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	}
	
	if (bHidePreviewMeshWhenHanded)
	{
		PreviewMesh->SetHiddenInGame(false);
	}
}

void AItemActor::OnRep_ItemObject()
{
	if (ItemObject)
	{
		OnInitializeItem();
	}
}

void AItemActor::OnRep_Handed()
{
	bHanded ? SetHandedMode() : SetFreeMode();
}

void AItemActor::UpdateItem() const
{
	PreviewMesh->SetStaticMesh(ItemObject->GetPreviewMesh());
}
