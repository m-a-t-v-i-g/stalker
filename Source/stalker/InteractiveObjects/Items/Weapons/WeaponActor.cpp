// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponActor.h"
#include "WeaponObject.h"
#include "Kismet/KismetSystemLibrary.h"

AWeaponActor::AWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWeaponActor::OnInitializeItem()
{
	Super::OnInitializeItem();

	if (auto WeaponObject = GetItemObject<UWeaponObject>())
	{
		WeaponParams = &WeaponObject->GetWeaponParams();
	}
}

void AWeaponActor::StartAttack()
{
	if (bHoldTrigger) return;

	bHoldTrigger = true;
	if (bCanAttack)
	{
		float AttackDelay = 1.0f / (WeaponParams->FireRate / 60.0f);
		FTimerDelegate CanAttackDelegate;
		CanAttackDelegate.BindLambda([&, this]
		{
			bCanAttack = true;
			GetWorldTimerManager().ClearTimer(CanAttackTimer);
		});
		GetWorldTimerManager().SetTimer(CanAttackTimer, CanAttackDelegate, AttackDelay, false);

		if (WeaponParams->bAutomatic)
		{
			FTimerDelegate RepeatAttackDelegate;
			RepeatAttackDelegate.BindLambda([&, this]
			{
				bCanAttack = true;
				CallAttack();
			});
			GetWorldTimerManager().SetTimer(RepeatAttackTimer, RepeatAttackDelegate, AttackDelay, true);
		}
		CallAttack();
		bCanAttack = false;
	}
}

void AWeaponActor::CallAttack()
{
	if (!CheckAttackAvailability())
	{
		StopAttack();
		return;
	}
	if (bCanAttack)
	{
		OnWeaponStartAttack.ExecuteIfBound();
		if (HasAuthority())
		{
			MulticastMakeAttackVisual();
		}
	}
}

void AWeaponActor::MulticastMakeAttackVisual_Implementation()
{
	UKismetSystemLibrary::PrintString(this, FString("ATTACK"), true, false, FLinearColor::Green);
}

void AWeaponActor::StopAttack()
{
	if (bHoldTrigger)
	{
		OnWeaponStopAttack.ExecuteIfBound();
		if (WeaponParams->bAutomatic)
		{
			GetWorldTimerManager().ClearTimer(RepeatAttackTimer);
		}
		bHoldTrigger = false;
	}
}

bool AWeaponActor::CheckAttackAvailability() const
{
	return GetItemObject<UWeaponObject>()->CanAttack();
}
