// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenPawn.h"
#include "BaseOrganic.generated.h"

class UCapsuleComponent;
class UOrganicAbilityComponent;

UCLASS()
class STALKER_API ABaseOrganic : public AGenPawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABaseOrganic(const FObjectInitializer& ObjectInitializer);

	static FName MeshName;
	static FName CapsuleName;
	static FName AbilitySystemComponentName;
	
	virtual void PossessedBy(AController* NewController) override;
	
	FORCEINLINE virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	template <class T>
	T* GetAbilitySystemComponent() const
	{
		return Cast<T>(GetAbilitySystemComponent());
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Organic")
	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return Mesh; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Organic")
	FORCEINLINE UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }

	template <class T>
	T* GetCapsuleComponent() const
	{
		return Cast<T>(GetCapsuleComponent());
	}
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	TObjectPtr<class UHealthAttributeSet> HealthAttributeSet;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UOrganicAbilityComponent> AbilitySystemComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<class UArrowComponent> ArrowComponent;
#endif
};
