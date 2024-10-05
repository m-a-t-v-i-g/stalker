// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenPawn.h"
#include "OrganicActorInterface.h"
#include "Library/OrganicLibrary.h"
#include "BaseOrganic.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJumped);

UCLASS()
class STALKER_API ABaseOrganic : public AGenPawn, public IOrganicActorInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABaseOrganic(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;
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

#pragma region Movement

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Organic|Movement")
	FDataTableRowHandle MovementTable;

	UPROPERTY(BlueprintReadOnly, Category = "Base Organic|Movement")
	FOrganicMovementModel MovementModel;

	UPROPERTY(VisibleInstanceOnly, Category = "Base Organic|Movement")
	EOrganicMovementState MovementState = EOrganicMovementState::None;

	UPROPERTY(VisibleInstanceOnly, Category = "Base Organic|Movement")
	EOrganicMovementState PrevMovementState = EOrganicMovementState::None;

#pragma endregion Movement

#pragma region Rotation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Organic|Rotation")
	EOrganicRotationMode InputRotationMode = EOrganicRotationMode::VelocityDirection;

	UPROPERTY(VisibleInstanceOnly, Replicated, Category = "Base Organic|Rotation")
	EOrganicRotationMode RotationMode = EOrganicRotationMode::VelocityDirection;
	
#pragma endregion Rotation

#pragma region Stance

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Base Organic|Stance")
	EOrganicStance InputStance = EOrganicStance::Standing;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Base Organic|Stance")
	EOrganicStance Stance = EOrganicStance::Standing;

#pragma endregion Stance

#pragma region Gait

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Base Organic|Gait")
	EOrganicGait InputGait = EOrganicGait::Medium;

	UPROPERTY(VisibleInstanceOnly, Category = "Base Organic|Gait")
	EOrganicGait Gait = EOrganicGait::Medium;

#pragma endregion Gait

#pragma region Overlay

	UPROPERTY(BlueprintReadOnly, Category = "Base Organic|Overlay")
	int32 OverlayOverrideState = 0;

#pragma endregion Overlay

public:
	UPROPERTY(BlueprintAssignable)
	FOnJumped OnJumpedDelegate;

	void OnMovementModeChanged();

#pragma region Movement
	
	void SetMovementModel();
	FOrganicMovementSettings GetMovementSettings() const;

	void SetMovementState(const EOrganicMovementState NewMovementState, bool bForce = false);
	FORCEINLINE EOrganicMovementState GetMovementState() const { return MovementState; }

	FORCEINLINE EOrganicMovementState GetPrevMovementState() const { return PrevMovementState; }

	void OnMovementStateChanged(const EOrganicMovementState PreviousState);

#pragma endregion Movement
	
#pragma region Rotation
	
	void SetInputRotationMode(EOrganicRotationMode NewInputRotationMode);
	FORCEINLINE EOrganicRotationMode GetInputRotationMode() const { return InputRotationMode; }

	UFUNCTION(Server, Reliable)
	void Server_SetInputRotationMode(EOrganicRotationMode NewInputRotationMode);
	
	void SetRotationMode(EOrganicRotationMode NewRotationMode, bool bForce = false);
	FORCEINLINE EOrganicRotationMode GetRotationMode() const { return RotationMode; }

	UFUNCTION(Server, Reliable)
	void Server_SetRotationMode(EOrganicRotationMode NewRotationMode, bool bForce);

	void OnRotationModeChanged(EOrganicRotationMode PrevRotationMode);

#pragma endregion Rotation
	
#pragma region Stance
	
	void SetInputStance(EOrganicStance NewInputStance);
	FORCEINLINE EOrganicStance GetInputStance() const { return InputStance; }
	
	UFUNCTION(Server, Reliable)
	void Server_SetInputStance(EOrganicStance NewInputStance);
	
	void SetStance(const EOrganicStance NewStance, bool bForce = false);
	FORCEINLINE EOrganicStance GetStance() const { return Stance; }

	virtual void OnStanceChanged(EOrganicStance PreviousStance);
	
#pragma endregion Stance
	
#pragma region Gait
	
	void SetInputGait(EOrganicGait NewInputGait);
	FORCEINLINE EOrganicGait GetInputGait() const { return InputGait; }

	void SetGait(const EOrganicGait NewGait, bool bForce = false);
	FORCEINLINE EOrganicGait GetGait() const { return Gait; }
	
	void OnGaitChanged(EOrganicGait PreviousGait);

#pragma endregion Gait
	
#pragma region Overlay

	void SetOverlayOverrideState(int32 NewState);
	FORCEINLINE int32 GetOverlayOverrideState() const { return OverlayOverrideState; }

#pragma endregion Overlay

	float GetAnimCurveValue(FName CurveName) const;
	
	void ForceUpdateCharacterState();

	void OnSprint(bool bEnabled);
	void OnCrouch();
	void OnUnCrouch();
	void OnJump();

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
