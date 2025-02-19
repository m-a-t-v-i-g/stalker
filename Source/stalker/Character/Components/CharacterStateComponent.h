﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterLibrary.h"
#include "StalkerCharacter.h"
#include "Components/ActorComponent.h"
#include "CharacterStateComponent.generated.h"

struct FOnAttributeChangeData;
class UHealthAttributeSet;
class UResistanceAttributeSet;
class UOrganicAbilityComponent;
class UStalkerCharacterMovementComponent;
class UCharacterWeaponComponent;
class UMovementModelConfig;
class AStalkerCharacter;

USTRUCT()
struct FCharacterRagdollData
{
	GENERATED_USTRUCT_BODY()

	ECollisionEnabled::Type CapsuleCollisionType = ECollisionEnabled::NoCollision;

	ECollisionEnabled::Type MeshCollisionType = ECollisionEnabled::NoCollision;

	ECollisionChannel CollisionChannel = ECC_MAX;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnRagdollStateChangedDelegate, bool);

UCLASS(ClassGroup = "Stalker", meta = (BlueprintSpawnableComponent))
class STALKER_API UCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterStateComponent();

	FOnRagdollStateChangedDelegate OnRagdollStateChangedDelegate;

	TMulticastDelegate<void()> OnCharacterDead;
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetupStateComponent(AStalkerCharacter* InCharacter);

	void SetMovementAction(ECharacterMovementAction NewAction, bool bForce = false);
	FORCEINLINE ECharacterMovementAction GetMovementAction() const { return MovementAction; }
	
	virtual void OnMovementActionChanged(ECharacterMovementAction PreviousAction);

	void SetOverlayState(ECharacterOverlayState NewState, bool bForce = false);
	FORCEINLINE ECharacterOverlayState GetOverlayState() const { return OverlayState; }

	virtual void OnOverlayStateChanged(ECharacterOverlayState PreviousState);
	
	void SetHealthState(ECharacterHealthState NewState, bool bForce = false);
	FORCEINLINE ECharacterHealthState GetHealthState() const { return HealthState; }

	virtual void OnHealthStateChanged(ECharacterHealthState PreviousState);
	
	void SetCombatState(ECharacterCombatState NewState, bool bForce = false);
	FORCEINLINE ECharacterCombatState GetCombatState() const { return CombatState; }

	virtual void OnCombatStateChanged(ECharacterCombatState PreviousState);
	
	bool IsAuthority() const;
	bool IsAutonomousProxy() const;
	bool IsSimulatedProxy() const;
	
protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Character State")
	ECharacterMovementAction MovementAction = ECharacterMovementAction::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = "OnRep_OverlayState", Category = "Character State")
	ECharacterOverlayState OverlayState = ECharacterOverlayState::Default;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = "OnRep_HealthState", Category = "Character State")
	ECharacterHealthState HealthState = ECharacterHealthState::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = "OnRep_CombatState", Category = "Character State")
	ECharacterCombatState CombatState = ECharacterCombatState::None;

	UPROPERTY(EditDefaultsOnly, Category = "Character State")
	float CombatStateTransitionTime = 3.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Character State")
	float NormalHealthStateLimit = 10.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Character State|Movement Models")
	TObjectPtr<const UMovementModelConfig> NormalMovementModel;
	
	UPROPERTY(EditDefaultsOnly, Category = "Character State|Movement Models")
	TObjectPtr<const UMovementModelConfig> InjuredMovementModel;

	virtual void InitializeComponent() override;

	UFUNCTION(BlueprintCallable, Category = "Character State|Ragdoll")
	void StartRagdoll();

	UFUNCTION(BlueprintCallable, Category = "Character State|Ragdoll")
	void StopRagdoll();
	
	void UpdateRagdoll(float DeltaSeconds);
	
	void SetupHealth();
	void OnMaxHealthChange(const FOnAttributeChangeData& HealthChangeData);
	void OnHealthChange(const FOnAttributeChangeData& HealthChangeData);
	
	void OnFireStart();
	void OnFireStop();
	void OnAimingStart();
	void OnAimingStop();
	void OnOverlayChanged(ECharacterOverlayState NewOverlay);

	void SetRelaxTimer();

	UFUNCTION()
	void OnRep_OverlayState(ECharacterOverlayState PrevOverlayState);

	UFUNCTION()
	void OnRep_HealthState(ECharacterHealthState PrevHealthState);

	UFUNCTION()
	void OnRep_CombatState(ECharacterCombatState PrevCombatState);

	USkeletalMeshComponent* GetCharacterMesh() const;

	UCapsuleComponent* GetCharacterCapsule() const;

private:
	TObjectPtr<AStalkerCharacter> CharacterRef;
	TObjectPtr<AController> ControllerRef;

	TObjectPtr<UStalkerCharacterMovementComponent> MovementComponentRef;
	TObjectPtr<UOrganicAbilityComponent> AbilityComponentRef;
	TObjectPtr<UCharacterWeaponComponent> WeaponComponentRef;

	TObjectPtr<const UHealthAttributeSet> HealthAttributeSet;
	TObjectPtr<const UResistanceAttributeSet> ResistanceAttributeSet;

	bool bRagdoll = false;
	
	FVector LastRagdollVelocity;
	
	bool bFiring = false;
	bool bAiming = false;
	
	FTimerHandle CombatStateTimer;
	
	bool bIsDead = false;

	FCharacterRagdollData PreRagdollData;
};
