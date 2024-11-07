// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractorInterface.h"

bool IInteractorInterface::ContainerInteract()
{
	return false;
}

bool IInteractorInterface::ItemInteract(const UItemObject* ItemObject)
{
	return false;
}
