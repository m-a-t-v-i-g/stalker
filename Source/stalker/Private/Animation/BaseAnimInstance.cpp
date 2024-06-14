// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/BaseAnimInstance.h"

void UBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	/*
	Character = Cast<AWCharacterBase>(TryGetPawnOwner());
	if (Character)
	{
		Character->OnJustJumpedDelegate.AddUniqueDynamic(this, &UBaseAnimInstance::OnJumped);
	}
	*/
	
}

void UBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}
