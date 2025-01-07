// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GenPawn.h"
#include "BaseOrganic.generated.h"

struct FInputActionValue;
struct FInputActionInstance;
class UCapsuleComponent;
class UOrganicAbilityComponent;
class UHitScanComponent;
class UInputMappingContext;
class UInputConfig;
class UAbilitySet;

UCLASS(Abstract)
class STALKER_API ABaseOrganic : public AGenPawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABaseOrganic(const FObjectInitializer& ObjectInitializer);

	static FName MeshName;
	static FName CapsuleName;
	static FName AbilitySystemComponentName;
	static FName HitScanComponentName;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;
	
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
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Organic")
	FORCEINLINE UHitScanComponent* GetHitScanComponent() const { return HitScanComponent; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<const UInputConfig> InputConfig;

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	TObjectPtr<class UHealthAttributeSet> HealthAttributeSet;

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	TObjectPtr<class UResistanceAttributeSet> ResistanceAttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TObjectPtr<const UAbilitySet> AbilitySet;

	virtual void PostInitializeComponents() override;
	
	virtual void BindDirectionalInput(UInputComponent* PlayerInputComponent) override;
	virtual void BindViewInput(UInputComponent* PlayerInputComponent) override;
	virtual void BindKeyInput(UInputComponent* PlayerInputComponent) override;

	virtual void InitEssentialComponents();
	virtual void InitEssentialData();
	virtual void InitLocalData();

	virtual void Moving(const FInputActionValue& Value);
	virtual void Viewing(const FInputActionValue& Value);

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UOrganicAbilityComponent> AbilitySystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHitScanComponent> HitScanComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<class UArrowComponent> ArrowComponent;
#endif
};
