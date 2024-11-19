// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "InteractorInterface.h"
#include "ItemObject.h"
#include "ItemSystemCore.h"
#include "Components/PawnInteractionComponent.h"
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
	SetReplicatingMovement(true);
}

void AItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ItemObject,	COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, bHanded,		COND_OwnerOnly)
}

void AItemActor::Destroyed()
{
	UnbindItemObject();
	Super::Destroyed();
}

bool AItemActor::OnInteract(AActor* Interactor)
{
	check(Interactor);
	
	if (ItemObject && ItemObject->CanCollected())
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

void AItemActor::SetEquipped()
{
	bHanded = true;
	PhysicCollision->SetSimulatePhysics(false);
	//InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AItemActor::SetGrounded()
{
	bHanded = false;
	PhysicCollision->SetSimulatePhysics(true);
	//InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
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

void AItemActor::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                 const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (auto InteractionComp = OtherActor->GetComponentByClass<UPawnInteractionComponent>())
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
		if (auto InteractionComp = OtherActor->GetComponentByClass<UPawnInteractionComponent>())
		{
			InteractionComp->RemovePossibleInteraction(this);
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

void AItemActor::OnRep_Handed()
{
	bHanded ? SetEquipped() : SetGrounded();
}
