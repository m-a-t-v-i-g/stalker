// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "StalkerCharacter.generated.h"

UCLASS()
class STALKER_API AStalkerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AStalkerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PreInitializeComponents() override;
	virtual void PossessedBy(AController* NewController) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<class UCharacterInventoryComponent> InventoryComponent;
	
public:
	FORCEINLINE UCharacterInventoryComponent* GetInventoryComponent() const { return InventoryComponent.Get(); }
};
