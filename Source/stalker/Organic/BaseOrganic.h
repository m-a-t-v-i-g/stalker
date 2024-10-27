// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenPawn.h"
#include "OrganicActorInterface.h"
#include "BaseOrganic.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJumped);

UCLASS()
class STALKER_API ABaseOrganic : public AGenPawn, public IOrganicActorInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABaseOrganic(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	virtual void PossessedBy(AController* NewController) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	template <class T>
	T* GetAbilitySystemComponent() const
	{
		return Cast<T>(GetAbilitySystemComponent());
	}
	
	static FName MeshName;
	static FName OrganicMovementName;
	static FName CapsuleName;
	static FName AbilitySystemComponentName;
	static FName InventoryComponentName;
	static FName WeaponComponentName;

protected:
	virtual void Tick(float DeltaSeconds) override;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOrganicMovementComponent> OrganicMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCapsuleComponent> CapsuleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStalkerAbilityComponent> AbilitySystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInventoryComponent> InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UWeaponComponent> WeaponComponent;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<class UArrowComponent> ArrowComponent;
#endif

protected:
	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	TObjectPtr<class UOrganicAttributeSet> OrganicAttributeSet;

	UPROPERTY(EditAnywhere, Category = "Movement Model")
	TObjectPtr<class UMovementModelConfig> DefaultMovementModel;

public:
	UPROPERTY(BlueprintAssignable)
	FOnJumped OnJumpedDelegate;

	bool IsSprinting() const;
	bool IsAirborne() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return Mesh; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE UOrganicMovementComponent* GetOrganicMovement() const { return OrganicMovement; }
	
	template <class T>
	T* GetOrganicMovement() const
	{
		return Cast<T>(GetOrganicMovement());
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }

	template <class T>
	T* GetCapsuleComponent() const
	{
		return Cast<T>(GetCapsuleComponent());
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	template <class T>
	T* GetInventoryComponent() const
	{
		return Cast<T>(GetInventoryComponent());
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE UWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }

	template <class T>
	T* GetWeaponComponent() const
	{
		return Cast<T>(GetWeaponComponent());
	}
};
