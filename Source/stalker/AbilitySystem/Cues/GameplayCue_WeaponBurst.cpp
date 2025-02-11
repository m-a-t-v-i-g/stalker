// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayCue_WeaponBurst.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/WeaponActor.h"
#include "Weapons/WeaponObject.h"

bool UGameplayCue_WeaponBurst::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (const UWeaponObject* WeaponObject = Cast<const UWeaponObject>(Parameters.SourceObject))
	{
		if (UNiagaraSystem* MuzzleEffect = WeaponObject->GetMuzzleEffect())
		{
			if (const AWeaponActor* WeaponActor = Cast<const AWeaponActor>(WeaponObject->GetBoundActor()))
			{
				UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleEffect, WeaponActor->GetMuzzle(), NAME_None,
				                                             FVector::ZeroVector, FRotator::ZeroRotator,
				                                             EAttachLocation::Type::SnapToTarget, true);
			}
		}
		
		if (USoundBase* FireSound = WeaponObject->GetFireSound())
		{
			if (const AWeaponActor* WeaponActor = Cast<const AWeaponActor>(WeaponObject->GetBoundActor()))
			{
				UGameplayStatics::SpawnSoundAttached(FireSound, WeaponActor->GetMuzzle(), NAME_None);
			}
		}
		
		if (const APawn* MyPawn = Cast<const APawn>(MyTarget))
		{
			auto PC = MyPawn->GetController<APlayerController>();
			if (PC && PC->PlayerCameraManager)
			{
				PC->PlayerCameraManager->StartCameraShake(WeaponObject->GetCameraShake());
			}
		}
	}
	
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
