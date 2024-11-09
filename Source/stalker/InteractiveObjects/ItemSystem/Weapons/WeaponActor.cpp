// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponActor.h"
#include "WeaponObject.h"
#include "Ammo/AmmoObject.h"
#include "Kismet/KismetSystemLibrary.h"

AWeaponActor::AWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWeaponActor::OnBindItem()
{
	Super::OnBindItem();

	if (auto WeaponObject = GetItemObject<UWeaponObject>())
	{
		WeaponParams = &WeaponObject->GetWeaponParams();
	}
}

void AWeaponActor::StartAttack()
{
	if (bHoldTrigger)
	{
		return;
	}

	if (bInFireRate)
	{
		CallAttack();
		WeaponParams->bAutomatic ? SetRepetitiveFireTimer() : SetSingleFireTimer();
	}
	
	bHoldTrigger = true;
}

void AWeaponActor::CallAttack()
{
	if (!CheckAttackAvailability())
	{
		StopAttack();
	}
	else if (bInFireRate)
	{
		OnWeaponStartAttack.ExecuteIfBound();
		
		if (HasAuthority())
		{
			MulticastMakeAttackVisual();
		}
		
		bInFireRate = false;
	}
}

void AWeaponActor::MulticastMakeAttackVisual_Implementation()
{
	UKismetSystemLibrary::PrintString(this, FString("ATTACK"), true, false, FLinearColor::Green);
}

void AWeaponActor::StopAttack()
{
	if (!bHoldTrigger)
	{
		return;
	}

	OnWeaponStopAttack.ExecuteIfBound();
	
	if (WeaponParams->bAutomatic)
	{
		GetWorldTimerManager().ClearTimer(RepeatAttackTimer);
		SetSingleFireTimer();
	}
	
	bHoldTrigger = false;
}

void AWeaponActor::StartAlternative()
{
}

void AWeaponActor::StopAlternative()
{
}

bool AWeaponActor::CheckAttackAvailability() const
{
	return GetItemObject<UWeaponObject>()->CanAttack();
}

bool AWeaponActor::IsAmmoAvailable(const UClass* AmmoClass) const
{
	return WeaponParams->AmmoClasses.Contains(AmmoClass);
}

void AWeaponActor::SetSingleFireTimer()
{
	if (!CanAttackTimer.IsValid())
	{
		FTimerDelegate CanAttackDelegate;
		CanAttackDelegate.BindLambda([&, this]
		{
			bInFireRate = true;
			GetWorldTimerManager().ClearTimer(CanAttackTimer);
		});
		
		GetWorldTimerManager().SetTimer(CanAttackTimer, CanAttackDelegate, CalculateFireRate(), false);
	}
}

void AWeaponActor::SetRepetitiveFireTimer()
{
	FTimerDelegate RepeatAttackDelegate;
	RepeatAttackDelegate.BindLambda([&, this]
	{
		bInFireRate = true;
		CallAttack();
	});
	
	GetWorldTimerManager().SetTimer(RepeatAttackTimer, RepeatAttackDelegate, CalculateFireRate(), true);
}

float AWeaponActor::GetDefaultFireRate() const
{
	return 1.0f / (WeaponParams->FireRate / 60.0f);
}

float AWeaponActor::CalculateFireRate() const
{
	return GetDefaultFireRate();
}
