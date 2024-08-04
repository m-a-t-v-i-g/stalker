// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "Components/SphereComponent.h"

FName AItemActor::SphereComponentName {"ItemCollision"};

AItemActor::AItemActor()
{
	SphereComponent = CreateDefaultSubobject<USphereComponent>(SphereComponentName);
	SetRootComponent(SphereComponent);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(GetRootComponent());
	
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMesh");
	SkeletalMesh->SetupAttachment(GetRootComponent());
	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AItemActor::InitItem(UItemObject* NewItemObject)
{
	check(NewItemObject);
	
	ItemObject = NewItemObject;
}
