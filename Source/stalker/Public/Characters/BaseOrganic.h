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

#pragma region Movement

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Organic|Movement")
	FDataTableRowHandle MovementTable;

	UPROPERTY(BlueprintReadOnly, Category = "Base Organic|Movement")
	FOrganicMovementStateSettings MovementModel;

	UPROPERTY(VisibleInstanceOnly, Category = "Base Organic|Movement")
	EOrganicMovementState MovementState = EOrganicMovementState::None;

	UPROPERTY(VisibleInstanceOnly, Category = "Base Organic|Movement")
	EOrganicMovementState PrevMovementState = EOrganicMovementState::None;

#pragma endregion Movement

#pragma region Rotation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Base Organic|Rotation")
	EOrganicRotationMode InputRotationMode = EOrganicRotationMode::VelocityDirection;

	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = "OnRep_RotationMode", Category = "Base Organic|Rotation")
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

	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	FVector Acceleration;

	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	FRotator ViewRotation;
	
	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	FRotator TargetRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	float ViewYawRate = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Base Organic")
	float ViewTurnLimit = 35.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Base Organic")
	FRotator AirborneRotation;

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

	UFUNCTION()
	void OnRep_RotationMode(EOrganicRotationMode PrevRotationMode);

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
	
	void GaitTick();
	
	void SetInputGait(EOrganicGait NewInputGait);
	FORCEINLINE EOrganicGait GetInputGait() const { return InputGait; }

	void SetGait(const EOrganicGait NewGait, bool bForce = false);
	FORCEINLINE EOrganicGait GetGait() const { return Gait; }
	
	void OnGaitChanged(EOrganicGait PreviousGait);

	EOrganicGait CalculateAllowedGait() const;
	EOrganicGait CalculateActualGait(EOrganicGait AllowedGait) const;

#pragma endregion Gait
	
	void ForceUpdateCharacterState();

	float GetAnimCurveValue(FName CurveName) const;
	
	bool CanBeFaster() const;
	
	virtual void UpdateGroundRotation(float DeltaTime);
	virtual void UpdateAirborneRotation(float DeltaTime);

	void SmoothRotation(const FRotator& Target, float TargetInterpSpeed, float ActorInterpSpeed, float DeltaTime);
	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime);
	
	float CalculateGroundRotationRate() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE FVector GetAcceleration() const { return Acceleration; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE FRotator GetOrganicViewRotation() const { return ViewRotation; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE float GetViewYawRate() const { return ViewYawRate; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE float GetSpeed() const { return Speed; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE float GetMovementInputAmount() const { return MovementInputAmount; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE bool HasMovementInput() const { return bHasMovementInput; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Base Organic")
	FORCEINLINE bool IsMoving() const { return bIsMoving; }
};
