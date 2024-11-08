// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "InteractorInterface.h"
#include "ItemObject.h"
#include "ItemSystemCore.h"
#include "Components/InteractionComponent.h"
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

	DOREPLIFETIME(AItemActor, ItemObject);
	DOREPLIFETIME_CONDITION(AItemActor, bHanded,	COND_OwnerOnly)
}

void AItemActor::Destroyed()
{
	UnbindItemObject();
	
	Super::Destroyed();
}

bool AItemActor::OnInteract(AActor* Interactor)
{
	if (ItemObject && Interactor)
	{
		if (auto InteractorInterface = Cast<IInteractorInterface>(Interactor))
		{
			if (InteractorInterface->ItemInteract(ItemObject))
			{
				Destroy();
				return true;
			}
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

void AItemActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemActor::OnInteractionSphereBeginOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AItemActor::OnInteractionSphereEndOverlap);
}

void AItemActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (!IsValid(ItemObject))
		{
			UItemPredictedData* PreData = bUsePredictedData ? PredictedData : nullptr;
			if (UItemObject* NewItemObject = UItemSystemCore::GenerateItemObject(GetWorld(), ItemDefinition, PreData))
			{
				ItemObject = NewItemObject;
				BindItemObject(ItemObject);
			}
		}
	}
}

void AItemActor::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                 const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (auto InteractionComp = OtherActor->GetComponentByClass<UInteractionComponent>())
		{
			InteractionComp->AddPossibleInteraction(this);
		}
	}
}

void AItemActor::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		if (auto InteractionComp = OtherActor->GetComponentByClass<UInteractionComponent>())
		{
			InteractionComp->RemovePossibleInteraction(this);
		}
	}
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
