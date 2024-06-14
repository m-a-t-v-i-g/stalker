// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenPawn.h"
#include "Library/OrganicLibrary.h"
#include "BaseOrganic.generated.h"

UCLASS()
class STALKER_API ABaseOrganic : public AGenPawn
{
	GENERATED_BODY()

public:
	ABaseOrganic();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	static FName CapsuleName;
	static FName OrganicMovementName;
	static FName MeshName;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UOrganicMovementComponent> OrganicMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Organic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UGenCapsuleComponent> CapsuleComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<class UArrowComponent> ArrowComponent;
#endif
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return Mesh; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE UOrganicMovementComponent* GetOrganicMovement() const { return OrganicMovement; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE UGenCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }

protected:
	EOrganicMovementState MovementState = EOrganicMovementState::None;

	EOrganicMovementState PrevMovementState = EOrganicMovementState::None;

#pragma region Gait

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character|Gait")
	EOrganicGait InputGait = EOrganicGait::Running;

	UPROPERTY(VisibleInstanceOnly, Category = "Base Organic")
	EOrganicGait Gait = EOrganicGait::Running;

#pragma endregion Gait

	EOrganicRotationMode RotationMode = EOrganicRotationMode::VelocityDirection;
	
	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	FVector Acceleration;

	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	FRotator ViewRotation;
	
	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	float ViewYawRate = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	FRotator LastVelocityRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	FRotator LastInputRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	float Speed = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	float MovementInputAmount = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	bool bHasMovementInput = false;

	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	bool bIsMoving = false;

	FVector PreviousVelocity;
	float PreviousViewYaw;

public:
	virtual void OrganicTick(float DeltaTime);

	void OnMovementModeChanged();

protected:
	void SetOrganicMovementState(const EOrganicMovementState NewState, bool bForce = false);
	void OnMovementStateChanged(const EOrganicMovementState PreviousState);
	
	void SetOrganicStance(const EOrganicStance NewStance, bool bForce = false);
	
	void SetOrganicGait(const EOrganicGait NewGait, bool bForce = false);

	virtual void UpdateGroundRotation(float DeltaTime);
	virtual void UpdateAirborneRotation(float DeltaTime);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE FVector GetAcceleration() const { return Acceleration; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE FRotator GetOrganicViewRotation() const { return ViewRotation; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE float GetViewYawRate() const { return ViewYawRate; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character|Movement")
	FORCEINLINE float GetSpeed() const { return Speed; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character|Movement")
	FORCEINLINE float GetMovementInputAmount() const { return MovementInputAmount; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character|Movement")
	FORCEINLINE bool HasMovementInput() const { return bHasMovementInput; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character|Movement")
	FORCEINLINE bool IsMoving() const { return bIsMoving; }
};
