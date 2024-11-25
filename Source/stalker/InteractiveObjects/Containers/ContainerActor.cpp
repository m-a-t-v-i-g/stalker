// Fill out your copyright notice in the Description page of Project Settings.

#include "ContainerActor.h"
#include "InteractorInterface.h"
#include "Components/InventoryComponent.h"
#include "DamageSystem/DamageType_Bullet.h"

FName AContainerActor::InventoryComponentName {"Inventory Component"};

AContainerActor::AContainerActor()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(InventoryComponentName);
	
	PrimaryActorTick.bCanEverTick = false;
}

bool AContainerActor::OnInteract(AActor* Interactor)
{
	if (Interactor)
	{
		if (auto InteractorInterface = Cast<IInteractorInterface>(Interactor))
		{
			InteractorInterface->InteractWithContainer(InventoryComponent);
			return true;
		}
	}
	return false;
}

void AContainerActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	OnTakePointDamage.AddDynamic(this, &AContainerActor::OnTakeDamage);
}

void AContainerActor::OnTakeDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy,
	FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection,
	const UDamageType* DamageType, AActor* DamageCauser)
{
	Cast<UDamageType_Bullet>(DamageType)->
}
