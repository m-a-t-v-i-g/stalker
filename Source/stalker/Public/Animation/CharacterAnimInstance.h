// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/OrganicAnimInstance.h"
#include "CharacterAnimInstance.generated.h"

UCLASS()
class STALKER_API UCharacterAnimInstance : public UOrganicAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Read Only Data|Character Information")
	TObjectPtr<class ABaseCharacter> Character = nullptr;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FCharacterOverlayState OverlayState = ECharacterOverlayState::Default;
};
